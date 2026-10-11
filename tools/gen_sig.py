"""Génère une signature d'octets minimale et unique pour des fonctions de Mewgenics.exe.

Équivalent Python (sans IDA) de tools/ida/gen_sigs.py, mêmes règles :
  - on part du début de la fonction et on ajoute une instruction à la fois ;
  - les déplacements rel32 (call/jmp/jcc), les accès mémoire relatifs à RIP et les
    immédiats qui pointent dans l'image deviennent des jokers « ? » ;
  - on s'arrête dès que le motif n'apparaît qu'une seule fois dans .text.

Usage : python tools/gen_sig.py <Mewgenics.exe> <rva_hex>[:NOM] ...
Sortie : une ligne par cible, au format de src/core/mgmp_sigs.generated.h.
"""
import sys
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from capstone import x86 as cx86


def load(path):
    pe = pefile.PE(path, fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    size = pe.OPTIONAL_HEADER.SizeOfImage
    text = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    return pe, base, size, text.VirtualAddress, text.get_data()


def masked_insn(ins, base, size):
    raw = list(ins.bytes)
    mask = [True] * len(raw)
    # déplacement relatif d'un saut ou d'un appel (cible = adresse de code)
    if ins.group(cx86.X86_GRP_JUMP) or ins.group(cx86.X86_GRP_CALL):
        for op in ins.operands:
            if op.type == cx86.X86_OP_IMM:
                n = ins.imm_size or 4
                off = len(raw) - n
                for i in range(off, len(raw)):
                    mask[i] = False
    for op in ins.operands:
        if op.type == cx86.X86_OP_MEM and op.mem.base == cx86.X86_REG_RIP:
            off, n = ins.disp_offset, ins.disp_size
            for i in range(off, off + n):
                mask[i] = False
        if op.type == cx86.X86_OP_IMM and ins.imm_size >= 4:
            v = op.imm & 0xFFFFFFFFFFFFFFFF
            if base <= v < base + size:
                off, n = ins.imm_offset, ins.imm_size
                for i in range(off, off + n):
                    mask[i] = False
    return raw, mask


def count_matches(data, raw, mask, limit=2):
    # ancre : la plus longue suite d'octets fixes
    best, cur, start = (0, 0), 0, 0
    for i, m in enumerate(mask):
        if m:
            if cur == 0:
                start = i
            cur += 1
            if cur > best[1]:
                best = (start, cur)
        else:
            cur = 0
    a_off, a_len = best
    anchor = bytes(raw[a_off:a_off + a_len])
    hits, pos = 0, data.find(anchor)
    while pos != -1:
        s = pos - a_off
        if 0 <= s and s + len(raw) <= len(data):
            if all((not mask[i]) or data[s + i] == raw[i] for i in range(len(raw))):
                hits += 1
                if hits >= limit:
                    return hits
        pos = data.find(anchor, pos + 1)
    return hits


def make_sig(path, rva):
    pe, base, size, tva, data = load(path)
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    off = rva - tva
    raw, mask = [], []
    for ins in md.disasm(data[off:off + 512], base + rva):
        r, m = masked_insn(ins, base, size)
        raw += r
        mask += m
        if sum(mask) >= 8 and count_matches(data, raw, mask) == 1:
            return " ".join(f"{b:02X}" if k else "?" for b, k in zip(raw, mask))
    return None


if __name__ == "__main__":
    exe = sys.argv[1]
    for spec in sys.argv[2:]:
        rva_s, _, name = spec.partition(":")
        rva = int(rva_s, 16)
        sig = make_sig(exe, rva)
        print(f'    {{ 0x{rva:08X}, "{name or hex(rva)}",\n      "{sig}" }},' if sig else f"    // {name}: aucune signature unique trouvee")
