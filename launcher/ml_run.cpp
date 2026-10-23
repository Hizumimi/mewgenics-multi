// ml_run.cpp -- voir ml_run.h.
#include "ml_run.h"

namespace ml {
namespace {

constexpr size_t kKeepLines = 200;   // loader.txt a tout ; l'interface, la fin

void release_job(HANDLE job) {
    // Le jeu reste dans le job apres le loader : sans KILL_ON_JOB_CLOSE, fermer
    // notre poignee ne le tue plus.
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION li = {};
    SetInformationJobObject(job, JobObjectExtendedLimitInformation, &li, sizeof(li));
    CloseHandle(job);
}

} // namespace

Launch::~Launch() { cancel(); }

bool Launch::start(const std::wstring& dir, const std::wstring& loader) {
    {
        std::lock_guard<std::mutex> lk(m_);
        if (snap_.state == State::Running) return false;
    }
    if (thread_.joinable()) thread_.join();   // le lancement d'avant, fini
    {
        std::lock_guard<std::mutex> lk(m_);
        snap_.state  = State::Running;
        snap_.phrase = u8"Lancement du mod…";
        snap_.lines.clear();
        snap_.cancelled = false;
        ++snap_.seq;
        report_     = LoaderReport();
        cancelled_  = false;
    }

    std::wstring exe = loader.empty() ? dir + L"\\mgmp_loader.exe" : loader;

    SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, TRUE };
    HANDLE rd = nullptr, wr = nullptr;
    if (!CreatePipe(&rd, &wr, &sa, 0)) {
        set_failed(u8"Windows n'a pas pu préparer le lancement. Relance Mewgenics Multi.");
        return true;
    }
    SetHandleInformation(rd, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags    = STARTF_USESTDHANDLES;
    si.hStdOutput = wr;
    si.hStdError  = wr;
    PROCESS_INFORMATION pi = {};
    std::wstring cmd = L"\"" + exe + L"\"";
    // En pause le temps de le mettre dans le job : le jeu qu'il demarre y sera aussi.
    BOOL ok = CreateProcessW(exe.c_str(), &cmd[0], nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr, dir.c_str(), &si, &pi);
    CloseHandle(wr);   // sinon le tuyau ne se ferme jamais a la sortie du loader
    if (!ok) {
        CloseHandle(rd);
        set_failed(u8"mgmp_loader.exe n'a pas pu démarrer : extrais de nouveau le dossier du mod depuis "
                   u8"le zip (ou ton antivirus l'a mis de côté).");
        return true;
    }

    // Sans job (deja dans un job qui l'interdit), seul le loader sera arrete.
    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (job) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION li = {};
        li.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &li, sizeof(li)) ||
            !AssignProcessToJobObject(job, pi.hProcess)) {
            CloseHandle(job);
            job = nullptr;
        }
    }
    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);

    // Comme le .bat : la sortie du loader garde dans loader.txt, a cote.
    HANDLE txt = CreateFileW((dir + L"\\loader.txt").c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                             nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    {
        std::lock_guard<std::mutex> lk(m_);
        job_  = job;
        proc_ = pi.hProcess;
    }
    thread_ = std::thread(&Launch::read_loop, this, rd, txt);
    return true;
}

void Launch::read_loop(HANDLE rd, HANDLE txt) {
    std::string pending;
    char buf[4096];
    DWORD n = 0;
    while (ReadFile(rd, buf, sizeof(buf), &n, nullptr) && n) {
        if (txt != INVALID_HANDLE_VALUE) {
            DWORD w = 0;
            WriteFile(txt, buf, n, &w, nullptr);
        }
        pending.append(buf, n);
        size_t nl;
        while ((nl = pending.find('\n')) != std::string::npos) {
            push_line(pending.substr(0, nl));
            pending.erase(0, nl + 1);
        }
    }
    if (!pending.empty()) push_line(pending);
    CloseHandle(rd);
    if (txt != INVALID_HANDLE_VALUE) CloseHandle(txt);

    // Le tuyau est ferme : le loader sort (ou est sorti). Le job et le
    // processus sont repris sous le verrou, pour que cancel() ne s'en serve
    // plus pendant qu'on les ferme.
    HANDLE proc, job;
    {
        std::lock_guard<std::mutex> lk(m_);
        proc  = proc_;
        job   = job_;
        proc_ = job_ = nullptr;
    }
    WaitForSingleObject(proc, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess(proc, &code);
    CloseHandle(proc);
    if (job) release_job(job);

    std::lock_guard<std::mutex> lk(m_);
    if (cancelled_ && report_.state != LoaderReport::Loaded) {
        snap_.state  = State::Failed;
        snap_.phrase = u8"Lancement annulé.";
        snap_.cancelled = true;
    } else {
        report_.finish((int)code);
        snap_.state  = report_.state == LoaderReport::Loaded ? State::Loaded : State::Failed;
        snap_.phrase = report_.phrase;
    }
    ++snap_.seq;
}

void Launch::push_line(const std::string& raw) {
    std::string line = raw;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    line = loader_bytes_to_utf8(line);

    std::lock_guard<std::mutex> lk(m_);
    report_.feed(line);
    if (!report_.phrase.empty()) snap_.phrase = report_.phrase;
    if (report_.state == LoaderReport::Loaded) snap_.state = State::Loaded;
    if (report_.state == LoaderReport::Failed) snap_.state = State::Failed;
    snap_.lines.push_back(line);
    if (snap_.lines.size() > kKeepLines) snap_.lines.erase(snap_.lines.begin());
    ++snap_.seq;
}

void Launch::set_failed(const std::string& phrase) {
    std::lock_guard<std::mutex> lk(m_);
    snap_.state  = State::Failed;
    snap_.phrase = phrase;
    ++snap_.seq;
}

Launch::Snapshot Launch::snapshot() const {
    std::lock_guard<std::mutex> lk(m_);
    return snap_;
}

void Launch::cancel() {
    {
        std::lock_guard<std::mutex> lk(m_);
        cancelled_ = true;
        // Mod deja charge : seulement le loader (avec MGMP_WAIT il attendrait
        // la fin du jeu), le jeu reste.
        if (job_ && report_.state != LoaderReport::Loaded) TerminateJobObject(job_, 1);
        else if (proc_) TerminateProcess(proc_, 1);
    }
    if (thread_.joinable()) thread_.join();
}

} // namespace ml
