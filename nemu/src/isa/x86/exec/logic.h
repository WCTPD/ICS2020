#include "cc.h"

static inline def_EHelper(test) {
  // TODO();
  rtl_and(s, s0, ddest, dsrc1);
  rtl_set_OF(s, rz);
  rtl_set_CF(s, rz);
  rtl_update_ZFSF(s, s0, id_dest->width);
  print_asm_template2(test);
}

static inline def_EHelper(and) {
  // TODO();
  rtl_sext(s, t0, dsrc1, id_src1->width);
  rtl_and(s, ddest, ddest, t0);
  rtl_set_CF(s, rz);
  rtl_set_OF(s, rz);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(and);
}

static inline def_EHelper(xor) {
  // TODO();
  rtl_sext(s, t0, dsrc1, id_src1->width);
  rtl_xor(s, ddest, ddest, t0);
  rtl_set_CF(s, rz);
  rtl_set_OF(s, rz);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(xor);
}

static inline def_EHelper(or) {
  // TODO();
  rtl_sext(s, t0, dsrc1, id_src1->width);
  rtl_or(s, ddest, ddest, t0);
  rtl_set_CF(s, rz);
  rtl_set_OF(s, rz);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(or);
}

static inline def_EHelper(not) {
  // TODO();
  rtl_not(s, ddest, ddest);
  operand_write(s, id_dest, ddest);
  print_asm_template1(not);
}

static inline def_EHelper(sar) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_mv(s, s0, ddest);
  rtl_mv(s, s1, dsrc1);
  rtl_sar(s, ddest, s0, s1);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(sar);
}

static inline def_EHelper(shl) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_mv(s, s0, ddest);
  rtl_mv(s, s1, dsrc1);
  rtl_shl(s, ddest, s0, s1);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(shl);
}

static inline def_EHelper(shr) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_mv(s, s0, ddest);
  rtl_mv(s, s1, dsrc1);
  rtl_shr(s, ddest, s0, s1);
  rtl_update_ZFSF(s, ddest, id_dest->width);
  operand_write(s, id_dest, ddest);
  print_asm_template2(shr);
}

static inline def_EHelper(setcc) {
  uint32_t cc = s->opcode & 0xf;
  rtl_setcc(s, ddest, cc);
  operand_write(s, id_dest, ddest);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

static inline def_EHelper(rol) {
#define CASEl(i, len)                                                          \
  case (i):                                                                    \
    count = *dsrc1 & (len);                                                    \
    while (count != 0) {                                                       \
      tmpcf = (*ddest >> (len)) & 1;                                           \
      *ddest = (*ddest << 1) | tmpcf;                                          \
      count--;                                                                 \
    }                                                                          \
    break;

  uint32_t count = 0, tmpcf = 0;
  switch (id_dest->width) {
    CASEl(1, 7)
    CASEl(2, 15)
    CASEl(4, 31)
  }
  rtl_set_CF(s, &tmpcf);
  if (count == 1) {
    if (((*ddest >> 7) & 1) != tmpcf) {
      rtl_li(s, s0, 1);
      rtl_set_OF(s, s0);
    } else {
      rtl_li(s, s0, 0);
      rtl_set_OF(s, s0);
    }
  }
  operand_write(s, id_dest, ddest);
}

static inline def_EHelper(bsr) {
  if (*dsrc1 == 0) {
    rtl_li(s, s0, 1);
    rtl_set_ZF(s, s0);
  } else {
    uint32_t tmp = *ddest = id_src1->width * 8 - 1;
    rtl_li(s, s0, 0);
    rtl_set_ZF(s, s0);
    while (((*dsrc1 >> tmp) & 1) == 0) {
      tmp -= 1;
      *ddest = tmp;
    }
  }
}

static inline def_EHelper(ror) {
#define CASEr(i, len)                                                          \
  case (i):                                                                    \
    count = *dsrc1 & (len);                                                    \
    while (count != 0) {                                                       \
      tmpcf = *ddest & 1;                                                      \
      *ddest = (*ddest >> 1) | (tmpcf << len);                                 \
      count--;                                                                 \
    }                                                                          \
    break;

  uint32_t count = 0, tmpcf = 0;
  switch (id_dest->width) {
    CASEr(1, 7)
    CASEr(2, 15)
    CASEr(4, 31)
  }
  if (count == 1) {
    if (((*ddest >> 7) & 1) != ((*ddest >> 6) & 1)) {
      rtl_li(s, s0, 1);
      rtl_set_OF(s, s0);
    } else {
      rtl_li(s, s0, 0);
      rtl_set_OF(s, s0);
    }
  }
  operand_write(s, id_dest, ddest);
}

static inline def_EHelper(shld) {
  uint8_t shift_amt = *dsrc1 % 32;
  uint32_t inBits = *dsrc2;
  if (shift_amt != 0) {
    if (shift_amt < id_dest->width) {
      *s0 = ((*ddest >> (id_dest->width * 8 - shift_amt)) & 1);
      rtl_set_CF(s, s0);
    }
    *ddest = *ddest << shift_amt;
    *ddest |= ((inBits >> (id_src2->width - shift_amt)) & (1 << shift_amt));
    rtl_update_ZFSF(s, ddest, id_dest->width);
  }
}