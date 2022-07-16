#include "nes.h"
#include "error.h"
#include <assert.h>

#include <stdio.h>

#define R_IMM(op) op, _0_0_0_0_0_0_end
#define R_ZP(op) _B0_42_0_0_10_0 , op, _0_0_0_0_0_0_end
#define R_ZPX(op) _3_42_C8_40_0_0, _B0_42_0_0_10_0, op, _0_0_0_0_0_0_end
#define R_ZPY(op) _4_42_C8_40_0_0, _B0_42_0_0_10_0, op, _0_0_0_0_0_0_end
#define R_ABS(op) _10_0_0_0_0_0, _B0_12_0_0_10_0, op, _0_0_0_0_0_0_end
#define R_ABX(op) _3_80_C8_42_0_0, _INCDPH, _B0_12_0_0_10_0, op, _0_0_0_0_0_0_end
#define R_ABY(op) _4_80_C8_42_0_0, _INCDPH, _B0_12_0_0_10_0, op, _0_0_0_0_0_0_end
#define R_INDX(op)  _3_42_C8_40_0_0, _50_44_0_0_10_0, _10_42_0_0_10_0, _B0_22_0_0_10_0, op, _0_0_0_0_0_0_end
#define R_INDY(op) _B0_44_0_0_10_0, _4_C2_C8_42_10_0, _INCDPH, _B0_12_0_0_10_0, op, _0_0_0_0_0_0_end

#define RMW_ZP(op) _C0_42_0_0_10_0, op, _F5_42_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end
#define RMW_ZPX(op) _3_42_C8_40_0_0, _C0_42_0_0_10_0, op, _F5_42_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end
#define RMW_ABS(op) _10_0_0_0_0_0, _C0_12_0_0_10_0, op, _F5_12_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end
#define RMW_ABX(op) _3_80_C8_40_0_0, _11_12_8_58_0_0, _C0_12_0_0_10_0, op, _F5_12_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end

// In the event of an unofficial opcode, crash
void _unofficial(struct Nes* nes) {
    error(UNIMPLEMENTED, "Unimplemented instruction: $%02X", nes->ir);
    assert(0);
}

// BRK
micro_instr* brk[] = { _F9_57_0_0_10_0_brk, _F8_57_0_0_10_0_brk, _F7_57_0_0_10_0_brk, _80_74_0_0_0_0, _90_62_2_0_0_0, _0_8_0_6_0_0, _0_0_0_0_0_0_end };

// ALU Implied
micro_instr* lsr[] = { _A0_8_AD_24_0_0, _0_0_0_0_0_0_end };
micro_instr* asl[] = { _A0_8_AD_44_0_0, _0_0_0_0_0_0_end };
micro_instr* ror[] = { _A0_8_AD_2C_0_0, _0_0_0_0_0_0_end };
micro_instr* rol[] = { _A0_8_AD_54_0_0, _0_0_0_0_0_0_end };
micro_instr* inx[] = { _33_8_C_4C_0_0, _0_0_0_0_0_0_end };
micro_instr* dex[] = { _33_8_FC_44_0_0, _0_0_0_0_0_0_end };
micro_instr* tax[] = { _30_42_0_0_10_0, _0_0_0_0_0_0_end };
micro_instr* txa[] = { _A3_8_C_44_0_0, _0_0_0_0_0_0_end };
micro_instr* txs[] = { _23_8_8_44_0_0, _0_0_0_0_0_0_end };
micro_instr* tsx[] = { _32_8_C_44_0_0, _0_0_0_0_0_0_end };
micro_instr* iny[] = { _44_8_C_4C_0_0, _0_0_0_0_0_0_end };
micro_instr* dey[] = { _44_8_FC_44_0_0, _0_0_0_0_0_0_end };
micro_instr* tay[] = { _40_8_C_44_0_0, _0_0_0_0_0_0_end };
micro_instr* tya[] = { _A4_8_C_44_0_0, _0_0_0_0_0_0_end };

// Flag set/clear
micro_instr* clc[] = { _B0_8_9_4_0_0_carry, _0_0_0_0_0_0_end };
micro_instr* sec[] = { _B0_8_F9_4_0_0_carry, _0_0_0_0_0_0_end };
micro_instr* cli[] = { _B0_8_9_4_0_0_interrupt, _0_0_0_0_0_0_end };
micro_instr* sei[] = { _B0_8_F9_4_0_0_interrupt, _0_0_0_0_0_0_end };
micro_instr* clv[] = { _B0_8_9_4_0_0_overflow, _0_0_0_0_0_0_end };
micro_instr* cld[] = { _B0_8_9_4_0_0_decimal, _0_0_0_0_0_0_end };
micro_instr* sed[] = { _B0_8_F9_4_0_0_decimal, _0_0_0_0_0_0_end };

// Stack 
micro_instr* php[] = { _F7_57_0_0_10_0_bflag, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* plp[] = { _B0_56_0_0_0_0, _70_52_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* pha[] = { _F0_57_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* pla[] = {_B0_56_0_0_0_0, _B0_52_0_0_10_0, _A0_8_CC_4_0_0, _0_0_0_0_0_0_end };
micro_instr* rts[] = { _B0_56_0_0_0_0, _80_56_0_0_10_0, _90_52_0_0_10_0, _0_0_0_0_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* jsr[] = { _B0_52_0_0_10_0, _F9_57_0_0_10_0, _F8_57_0_0_10_0, _10_0_0_0_0_0, _B0_18_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* rti[] = { _B0_56_0_0_0_0, _70_56_0_0_10_0, _80_56_0_0_10_0, _90_52_0_0_10_0, _0_8_0_6_0_0, _0_0_0_0_0_0_end };
micro_instr* jmp_abs[] = { _10_0_0_0_0_0, _B0_18_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* jmp_ind[] = { _10_0_0_0_0_0, _80_14_0_0_0_0, _90_12_0_0_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };

// NOP
micro_instr* nop[] = { _0_8_0_4_0_0, _0_0_0_0_0_0_end };

// W Ops
micro_instr* sta_zp[] = { _F0_42_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* stx_zp[] = { _F3_42_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sty_zp[] = { _F4_42_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sta_zpx[] = { _3_42_C8_40_0_0, _F0_42_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sty_zpx[] = { _3_42_C8_40_0_0, _F4_42_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* stx_zpy[] = { _4_42_C8_40_0_0, _F3_42_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sta_abs[] = { _10_0_0_0_0_0, _F0_12_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* stx_abs[] = { _10_0_0_0_0_0, _F3_12_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sty_abs[] = { _10_0_0_0_0_0, _F4_12_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sta_abx[] = { _3_0_C8_40_0_0, _10_12_C8_18_0_0, _F0_12_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sta_aby[] = { _4_0_C8_40_0_0, _10_12_C8_18_0_0, _F0_12_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sta_indy[] = { _B0_44_0_0_10_0, _4_C2_C8_40_10_0, _11_12_8_58_0_0, _F0_12_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* sta_indx[] = { _3_42_C8_40_0_0, _50_44_0_0_10_0, _10_42_0_0_10_0, _F0_22_0_0_10_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };

// R Ops
micro_instr* lda_imm[] = { R_IMM(_A0_8_CC_4_0_0) };
micro_instr* ldx_imm[] = { R_IMM(_30_8_CC_4_0_0) };
micro_instr* ldy_imm[] = { R_IMM(_40_8_CC_4_0_0) };
micro_instr* adc_imm[] = { R_IMM(_A0_8_CE_55_0_0) };
micro_instr* sbc_imm[] = { R_IMM(_A0_8_3E_55_0_0) };
micro_instr* cmp_imm[] = { R_IMM(_B0_8_3D_4C_0_0) };
micro_instr* cpx_imm[] = { R_IMM(_B3_8_3D_4C_0_0) };
micro_instr* cpy_imm[] = { R_IMM(_B4_8_3D_4C_0_0) };
micro_instr* and_imm[] = { R_IMM(_A0_8_8C_4_0_0) };
micro_instr* ora_imm[] = { R_IMM(_A0_8_EC_4_0_0) };
micro_instr* eor_imm[] = { R_IMM(_A0_8_6C_4_0_0) };
micro_instr* bit_zp[] = {R_ZP(_B0_8_8F_34_0_0) };
micro_instr* lda_zp[] = {R_ZP(_A0_8_CC_4_0_0) };
micro_instr* ldx_zp[] = {R_ZP(_30_8_CC_4_0_0) };
micro_instr* ldy_zp[] = {R_ZP(_40_8_CC_4_0_0) };
micro_instr* adc_zp[] = {R_ZP(_A0_8_CE_55_0_0) };
micro_instr* sbc_zp[] = {R_ZP(_A0_8_3E_55_0_0) };
micro_instr* cmp_zp[] = {R_ZP(_B0_8_3D_4C_0_0) };
micro_instr* cpx_zp[] = {R_ZP(_B3_8_3D_4C_0_0) };
micro_instr* cpy_zp[] = {R_ZP(_B4_8_3D_4C_0_0) };
micro_instr* and_zp[] = {R_ZP(_A0_8_8C_4_0_0) };
micro_instr* ora_zp[] = {R_ZP(_A0_8_EC_4_0_0) };
micro_instr* eor_zp[] = {R_ZP(_A0_8_6C_4_0_0) };
micro_instr* lda_zpx[] = { R_ZPX(_A0_8_CC_4_0_0) };
micro_instr* ldy_zpx[] = { R_ZPX(_40_8_CC_4_0_0) };
micro_instr* adc_zpx[] = { R_ZPX(_A0_8_CE_55_0_0) };
micro_instr* sbc_zpx[] = { R_ZPX(_A0_8_3E_55_0_0) };
micro_instr* cmp_zpx[] = { R_ZPX(_B0_8_3D_4C_0_0) };
micro_instr* and_zpx[] = { R_ZPX(_A0_8_8C_4_0_0) };
micro_instr* ora_zpx[] = { R_ZPX(_A0_8_EC_4_0_0) };
micro_instr* eor_zpx[] = { R_ZPX(_A0_8_6C_4_0_0) };
micro_instr* ldx_zpy[] = { R_ZPY(_30_8_CC_4_0_0) };
micro_instr* bit_abs[] = { R_ABS(_B0_8_8F_34_0_0) };
micro_instr* lda_abs[] = { R_ABS(_A0_8_CC_4_0_0) };
micro_instr* ldx_abs[] = { R_ABS(_30_8_CC_4_0_0) };
micro_instr* ldy_abs[] = { R_ABS(_40_8_CC_4_0_0) };
micro_instr* adc_abs[] = { R_ABS(_A0_8_CE_55_0_0) };
micro_instr* sbc_abs[] = { R_ABS(_A0_8_3E_55_0_0) };
micro_instr* cmp_abs[] = { R_ABS(_B0_8_3D_4C_0_0) };
micro_instr* cpx_abs[] = { R_ABS(_B3_8_3D_4C_0_0) };
micro_instr* cpy_abs[] = { R_ABS(_B4_8_3D_4C_0_0) };
micro_instr* and_abs[] = { R_ABS(_A0_8_8C_4_0_0) };
micro_instr* ora_abs[] = { R_ABS(_A0_8_EC_4_0_0) };
micro_instr* eor_abs[] = { R_ABS(_A0_8_6C_4_0_0) };
micro_instr* lda_abx[] = { R_ABX(_A0_8_CC_4_0_0) };
micro_instr* ldy_abx[] = { R_ABX(_40_8_CC_4_0_0) };
micro_instr* adc_abx[] = { R_ABX(_A0_8_CE_55_0_0) };
micro_instr* sbc_abx[] = { R_ABX(_A0_8_3E_55_0_0) };
micro_instr* cmp_abx[] = { R_ABX(_B0_8_3D_4C_0_0) };
micro_instr* and_abx[] = { R_ABX(_A0_8_8C_4_0_0) };
micro_instr* ora_abx[] = { R_ABX(_A0_8_EC_4_0_0) };
micro_instr* eor_abx[] = { R_ABX(_A0_8_6C_4_0_0) };
micro_instr* lda_aby[] = { R_ABY(_A0_8_CC_4_0_0) };
micro_instr* ldx_aby[] = { R_ABY(_30_8_CC_4_0_0) };
micro_instr* adc_aby[] = { R_ABY(_A0_8_CE_55_0_0) };
micro_instr* sbc_aby[] = { R_ABY(_A0_8_3E_55_0_0) };
micro_instr* cmp_aby[] = { R_ABY(_B0_8_3D_4C_0_0) };
micro_instr* and_aby[] = { R_ABY(_A0_8_8C_4_0_0) };
micro_instr* ora_aby[] = { R_ABY(_A0_8_EC_4_0_0) };
micro_instr* eor_aby[] = { R_ABY(_A0_8_6C_4_0_0) };
micro_instr* lda_indx[] = { R_INDX(_A0_8_CC_4_0_0) };
micro_instr* adc_indx[] = { R_INDX(_A0_8_CE_55_0_0) };
micro_instr* sbc_indx[] = { R_INDX(_A0_8_3E_55_0_0) };
micro_instr* cmp_indx[] = { R_INDX(_B0_8_3D_4C_0_0) };
micro_instr* and_indx[] = { R_INDX(_A0_8_8C_4_0_0) };
micro_instr* ora_indx[] = { R_INDX(_A0_8_EC_4_0_0) };
micro_instr* eor_indx[] = { R_INDX(_A0_8_6C_4_0_0) };
micro_instr* lda_indy[] = { R_INDY(_A0_8_CC_4_0_0) };
micro_instr* adc_indy[] = { R_INDY(_A0_8_CE_55_0_0) };
micro_instr* sbc_indy[] = { R_INDY(_A0_8_3E_55_0_0) };
micro_instr* cmp_indy[] = { R_INDY(_B0_8_3D_4C_0_0) };
micro_instr* and_indy[] = { R_INDY(_A0_8_8C_4_0_0) };
micro_instr* ora_indy[] = { R_INDY(_A0_8_EC_4_0_0) };
micro_instr* eor_indy[] = { R_INDY(_A0_8_6C_4_0_0) };

// RMW Ops
micro_instr* lsr_zp[] = { RMW_ZP(_5B_32_CD_20_0_0_ml6502) };
micro_instr* asl_zp[] = { RMW_ZP(_5B_32_CD_80_0_0_ml6502) };
micro_instr* ror_zp[] = { RMW_ZP(_5B_32_CD_28_0_0_ml6502) };
micro_instr* rol_zp[] = { RMW_ZP(_5B_32_CD_90_0_0_ml6502) };
micro_instr* inc_zp[] = { RMW_ZP(_50_32_C_88_0_0_ml6502) };
micro_instr* dec_zp[] = { RMW_ZP(_50_32_FC_80_0_0_ml6502) };
micro_instr* lsr_zpx[] = { RMW_ZPX(_5B_32_CD_20_0_0_ml6502) };
micro_instr* asl_zpx[] = { RMW_ZPX(_5B_32_CD_80_0_0_ml6502) };
micro_instr* ror_zpx[] = { RMW_ZPX(_5B_32_CD_28_0_0_ml6502) };
micro_instr* rol_zpx[] = { RMW_ZPX(_5B_32_CD_90_0_0_ml6502) };
micro_instr* inc_zpx[] = { RMW_ZPX(_50_32_C_88_0_0_ml6502) };
micro_instr* dec_zpx[] = { RMW_ZPX(_50_32_FC_80_0_0_ml6502) };
micro_instr* lsr_abs[] = { RMW_ABS(_5B_32_CD_20_0_0_ml6502) };
micro_instr* asl_abs[] = { RMW_ABS(_5B_32_CD_80_0_0_ml6502) };
micro_instr* ror_abs[] = { RMW_ABS(_5B_32_CD_28_0_0_ml6502) };
micro_instr* rol_abs[] = { RMW_ABS(_5B_32_CD_90_0_0_ml6502) };
micro_instr* inc_abs[] = { RMW_ABS(_50_32_C_88_0_0_ml6502) };
micro_instr* dec_abs[] = { RMW_ABS(_50_32_FC_80_0_0_ml6502) };
micro_instr* lsr_abx[] = { RMW_ABX(_5B_32_CD_20_0_0_ml6502) };
micro_instr* asl_abx[] = { RMW_ABX(_5B_32_CD_80_0_0_ml6502) };
micro_instr* ror_abx[] = { RMW_ABX(_5B_32_CD_28_0_0_ml6502) };
micro_instr* rol_abx[] = { RMW_ABX(_5B_32_CD_90_0_0_ml6502) };
micro_instr* inc_abx[] = { RMW_ABX(_50_32_C_88_0_0_ml6502) };
micro_instr* dec_abx[] = { RMW_ABX(_50_32_FC_80_0_0_ml6502) };

// Branch
micro_instr* bpl[] = { _bpl, _99_2_C8_78_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* bmi[] = { _bmi, _99_2_C8_78_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* bvc[] = { _bvc, _99_2_C8_78_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* bvs[] = { _bvs, _99_2_C8_78_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* bcc[] = { _bcc, _99_2_C8_78_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* bcs[] = { _bcs, _99_2_C8_78_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* bne[] = { _bne, _99_2_C8_78_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };
micro_instr* beq[] = { _beq, _99_2_C8_78_0_0, _0_8_0_4_0_0, _0_0_0_0_0_0_end };

// Unofficial
micro_instr* unofficial[] = { _unofficial };

// Convert an opcode to its micro-code sequence
micro_instr** lookup[] = {
    /*00*/ brk, /*01*/ ora_indx, /*02*/ unofficial, /*03*/ unofficial, /*04*/ unofficial, /*05*/ ora_zp, /*06*/ asl_zp, /*07*/ unofficial, /*08*/ php, /*09*/ ora_imm, /*0A*/ asl, /*0B*/ unofficial, /*0C*/ unofficial, /*0D*/ ora_abs, /*0E*/ asl_abs, /*0F*/ unofficial,
    /*10*/ bpl, /*11*/ ora_indy, /*12*/ unofficial, /*13*/ unofficial, /*14*/ unofficial, /*15*/ ora_zpx, /*16*/ asl_zpx, /*17*/ unofficial, /*18*/ clc, /*19*/ ora_aby, /*1A*/ unofficial, /*1B*/ unofficial, /*1C*/ unofficial, /*1D*/ ora_abx, /*1E*/ asl_abx, /*1F*/ unofficial,
    /*20*/ jsr, /*21*/ and_indx, /*22*/ unofficial, /*23*/ unofficial, /*24*/ bit_zp, /*25*/ and_zp, /*26*/ rol_zp, /*27*/ unofficial, /*28*/ plp, /*29*/ and_imm, /*2A*/ rol, /*2B*/ unofficial, /*2C*/ bit_abs, /*2D*/ and_abs, /*2E*/ rol_abs, /*2F*/ unofficial,
    /*30*/ bmi, /*31*/ and_indy, /*32*/ unofficial, /*33*/ unofficial, /*34*/ unofficial, /*35*/ and_zpx, /*36*/ rol_zpx, /*37*/ unofficial, /*38*/ sec, /*39*/ and_aby, /*3A*/ unofficial, /*3B*/ unofficial, /*3C*/ unofficial, /*3D*/ and_abx, /*3E*/ rol_abx, /*3F*/ unofficial,
    /*40*/ rti, /*41*/ eor_indx, /*42*/ unofficial, /*43*/ unofficial, /*44*/ unofficial, /*45*/ eor_zp, /*46*/ lsr_zp, /*47*/ unofficial, /*48*/ pha, /*49*/ eor_imm, /*4A*/ lsr, /*4B*/ unofficial, /*4C*/ jmp_abs, /*4D*/ eor_abs, /*4E*/ lsr_abs, /*4F*/ unofficial,
    /*50*/ bvc, /*51*/ eor_indy, /*52*/ unofficial, /*53*/ unofficial, /*54*/ unofficial, /*55*/ eor_zpx, /*56*/ lsr_zpx, /*57*/ unofficial, /*58*/ cli, /*59*/ eor_aby, /*5A*/ unofficial, /*5B*/ unofficial, /*5C*/ unofficial, /*5D*/ eor_abx, /*5E*/ lsr_abx, /*5F*/ unofficial,
    /*60*/ rts, /*61*/ adc_indx, /*62*/ unofficial, /*63*/ unofficial, /*64*/ unofficial, /*65*/ adc_zp, /*66*/ ror_zp, /*67*/ unofficial, /*68*/ pla, /*69*/ adc_imm, /*6A*/ ror, /*6B*/ unofficial, /*6C*/ jmp_ind, /*6D*/ adc_abs, /*6E*/ ror_abs, /*6F*/ unofficial,
    /*70*/ bvs, /*71*/ adc_indy, /*72*/ unofficial, /*73*/ unofficial, /*74*/ unofficial, /*75*/ adc_zpx, /*76*/ ror_zpx, /*77*/ unofficial, /*78*/ sei, /*79*/ adc_aby, /*7A*/ unofficial, /*7B*/ unofficial, /*7C*/ unofficial, /*7D*/ adc_abx, /*7E*/ ror_abx, /*7F*/ unofficial,
    /*80*/ unofficial, /*81*/ sta_indx, /*82*/ unofficial, /*83*/ unofficial, /*84*/ sty_zp, /*85*/ sta_zp, /*86*/ stx_zp, /*87*/ unofficial, /*88*/ dey, /*89*/ unofficial, /*8A*/ txa, /*8B*/ unofficial, /*8C*/ sty_abs, /*8D*/ sta_abs, /*8E*/ stx_abs, /*8F*/ unofficial,
    /*90*/ bcc, /*91*/ sta_indy, /*92*/ unofficial, /*93*/ unofficial, /*94*/ sty_zpx, /*95*/ sta_zpx, /*96*/ stx_zpy, /*97*/ unofficial, /*98*/ tya, /*99*/ sta_aby, /*9A*/ txs, /*9B*/ unofficial, /*9C*/ unofficial, /*9D*/ sta_abx, /*9E*/ unofficial, /*9F*/ unofficial,
    /*A0*/ ldy_imm, /*A1*/ lda_indx, /*A2*/ ldx_imm, /*A3*/ unofficial, /*A4*/ ldy_zp, /*A5*/ lda_zp, /*A6*/ ldx_zp, /*A7*/ unofficial, /*A8*/ tay, /*A9*/ lda_imm, /*AA*/ tax, /*AB*/ unofficial, /*AC*/ ldy_abs, /*AD*/ lda_abs, /*AE*/ ldx_abs, /*AF*/ unofficial,
    /*B0*/ bcs, /*B1*/ lda_indy, /*B2*/ unofficial, /*B3*/ unofficial, /*B4*/ ldy_zpx, /*B5*/ lda_zpx, /*B6*/ ldx_zpy, /*B7*/ unofficial, /*B8*/ clv, /*B9*/ lda_aby, /*BA*/ tsx, /*BB*/ unofficial, /*BC*/ ldy_abx, /*BD*/ lda_abx, /*BE*/ ldx_aby, /*BF*/ unofficial,
    /*C0*/ cpy_imm, /*C1*/ cmp_indx, /*C2*/ unofficial, /*C3*/ unofficial, /*C4*/ cpy_zp, /*C5*/ cmp_zp, /*C6*/ dec_zp, /*C7*/ unofficial, /*C8*/ iny, /*C9*/ cmp_imm, /*CA*/ dex, /*CB*/ unofficial, /*CC*/ cpy_abs, /*CD*/ cmp_abs, /*CE*/ dec_abs, /*CF*/ unofficial,
    /*D0*/ bne, /*D1*/ cmp_indy, /*D2*/ unofficial, /*D3*/ unofficial, /*D4*/ unofficial, /*D5*/ cmp_zpx, /*D6*/ dec_zpx, /*D7*/ unofficial, /*D8*/ cld, /*D9*/ cmp_aby, /*DA*/ unofficial, /*DB*/ unofficial, /*DC*/ unofficial, /*DD*/ cmp_abx, /*DE*/ dec_abx, /*DF*/ unofficial,
    /*E0*/ cpx_imm, /*E1*/ sbc_indx, /*E2*/ unofficial, /*E3*/ unofficial, /*E4*/ cpx_zp, /*E5*/ sbc_zp, /*E6*/ inc_zp, /*E7*/ unofficial, /*E8*/ inx, /*E9*/ sbc_imm, /*EA*/ nop, /*EB*/ unofficial, /*EC*/ cpx_abs, /*ED*/ sbc_abs, /*EE*/ inc_abs, /*EF*/ unofficial,
    /*F0*/ beq, /*F1*/ sbc_indy, /*F2*/ unofficial, /*F3*/ unofficial, /*F4*/ unofficial, /*F5*/ sbc_zpx, /*F6*/ inc_zpx, /*F7*/ unofficial, /*F8*/ sed, /*F9*/ sbc_aby, /*FA*/ unofficial, /*FB*/ unofficial, /*FC*/ unofficial, /*FD*/ sbc_abx, /*FE*/ inc_abx, /*FF*/ unofficial,
};

micro_instr** get_micro(uint8_t op) {
    return lookup[op];
}

void step_cpu(struct Nes* nes) {
    (*(nes->instr_ptr++))(nes);

}

// A := A LSR 0; SETF(NZC); IR := *PC; PC += 1; END 
void _A0_8_AD_24_0_0(struct Nes* nes) {
    uint8_t bit0 = nes->acc & 0x01;
    nes->acc >>= 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, 0);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A := A ASL 0; SETF(NZC); IR := *PC; PC += 1; END 
void _A0_8_AD_44_0_0(struct Nes* nes) {
    uint8_t bit7 = nes->acc >> 7;
    nes->acc <<= 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit7);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A := A ROR 0; SETF(NZC); IR := *PC; PC += 1; END
void _A0_8_AD_2C_0_0(struct Nes* nes) {
    uint8_t bit0 = nes->acc & 0x01;
    nes->acc >>= 1;
    nes->acc |= get_flag(nes, STATUS_FLAG_CARRY) << 7; // bit7 = carry
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// T := 0 ROR B; SETF(NZC) 
void _5B_32_CD_28_0_0_ml6502(struct Nes* nes) {
    uint8_t bit0 = nes->b & 0x01;
    nes->t = nes->b >> 1;
    nes->t |= get_flag(nes, STATUS_FLAG_CARRY) << 7;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->t >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->t == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit0);
}

// A := A ROL 0; SETF(NZC); IR := *PC; PC += 1; END 
void _A0_8_AD_54_0_0(struct Nes* nes) {
    uint8_t bit7 = nes->acc >> 7;
    nes->acc <<= 1;
    nes->acc |= get_flag(nes, STATUS_FLAG_CARRY); // bit0 = carry
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit7);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// T := 0 ROL B; SETF(NZC) 
void _5B_32_CD_90_0_0_ml6502(struct Nes* nes) {
    uint8_t bit7 = nes->b >> 7;
    nes->t = nes->b << 1;
    nes->t |= get_flag(nes, STATUS_FLAG_CARRY);
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->t >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->t == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit7);
}

// T := B + 1; SETF(NZ) 
void _50_32_C_88_0_0_ml6502(struct Nes* nes) {
    nes->t = nes->b + 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->t >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->t == 0);
}

// T := B - 1; SETF(NZ) 
void _50_32_FC_80_0_0_ml6502(struct Nes* nes) {
    nes->t = nes->b - 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->t >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->t == 0);
}

// A := A ADC B; SETF(NZCV); IR := *PC; PC += 1; END.D 
void _A0_8_CE_55_0_0(struct Nes* nes) {
    uint8_t old_acc = nes->acc;
    uint16_t result = nes->acc + nes->b + get_flag(nes, STATUS_FLAG_CARRY);
    nes->acc = result;
    uint8_t overflow = ((old_acc ^ nes->acc) & (nes->b ^ nes->acc) & 0x80) > 0;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_OVERFLOW, overflow);
    set_flag(nes, STATUS_FLAG_CARRY, result > 0xFF);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A := A SBC B; SETF(NZCV); IR := *PC; PC += 1; END.D 
void _A0_8_3E_55_0_0(struct Nes* nes) {
    uint8_t val = ~nes->b;
    uint8_t old_acc = nes->acc;
    uint16_t result = nes->acc + val + get_flag(nes, STATUS_FLAG_CARRY);
    nes->acc = result;
    uint8_t overflow = ((old_acc ^ nes->acc) & (val ^ nes->acc) & 0x80) > 0;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_OVERFLOW, overflow);
    set_flag(nes, STATUS_FLAG_CARRY, result > 0xFF);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// X := X + 1; SETF(NZ); IR := *PC; PC += 1; END 
void _33_8_C_4C_0_0(struct Nes* nes) {
    nes->x += 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// X := X - 1; SETF(NZ); IR := *PC; PC += 1; END 
void _33_8_FC_44_0_0(struct Nes* nes) {
    nes->x -= 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// X := A; SETF(NZ); IR := *PC; PC += 1; END 
void _30_42_0_0_10_0(struct Nes* nes) {
    nes->x = nes->acc;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A := X; SETF(NZ); IR := *PC; PC += 1; END 
void _A3_8_C_44_0_0(struct Nes* nes) {
    nes->acc = nes->x;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// SP := X; IR := *PC; PC += 1; END 
void _23_8_8_44_0_0(struct Nes* nes) {
    nes->sp = nes->x;
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// X := SP; SETF(NZ); IR := *PC; PC += 1; END 
void _32_8_C_44_0_0(struct Nes* nes) {
    nes->x = nes->sp;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// Y := Y + 1; SETF(NZ); IR := *PC; PC += 1; END 
void _44_8_C_4C_0_0(struct Nes* nes) {
    nes->y += 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// Y := Y - 1; SETF(NZ); IR := *PC; PC += 1; END 
void _44_8_FC_44_0_0(struct Nes* nes) {
    nes->y -= 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// Y := A; SETF(NZ); IR := *PC; PC += 1; END 
void _40_8_C_44_0_0(struct Nes* nes) {
    nes->y = nes->acc;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A := Y; SETF(NZ); IR := *PC; PC += 1; END 
void _A4_8_C_44_0_0(struct Nes* nes) {
    nes->acc = nes->y;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}


// A := A EOR B; SETF(NZ); IR := *PC; PC += 1; END 
void _A0_8_6C_4_0_0(struct Nes* nes) {
    nes->acc ^= nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A := A OR B; SETF(NZ); IR := *PC; PC += 1; END 
void _A0_8_EC_4_0_0(struct Nes* nes) {
    nes->acc |= nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A CMP B; SETF(NZC); IR := *PC; PC += 1; END 
void _B0_8_3D_4C_0_0(struct Nes* nes) {
    uint8_t t = nes->acc - nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, t >= 0x80);
    set_flag(nes, STATUS_FLAG_CARRY, nes->acc >= nes->b);
    set_flag(nes, STATUS_FLAG_ZERO, t == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// X CMP B; SETF(NZC); IR := *PC; PC += 1; END 
void _B3_8_3D_4C_0_0(struct Nes* nes) {
    uint8_t t = nes->x - nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, t >= 0x80);
    set_flag(nes, STATUS_FLAG_CARRY, nes->x >= nes->b);
    set_flag(nes, STATUS_FLAG_ZERO, t == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;   
}

// Y CMP B; SETF(NZC); IR := *PC; PC += 1; END 
void _B4_8_3D_4C_0_0(struct Nes* nes) {
    uint8_t t = nes->y - nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, t >= 0x80);
    set_flag(nes, STATUS_FLAG_CARRY, nes->y >= nes->b);
    set_flag(nes, STATUS_FLAG_ZERO, t == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;       
}

// A := A AND B; SETF(NZ); IR := *PC; PC += 1; END 
void _A0_8_8C_4_0_0(struct Nes* nes) {
    nes->acc &= nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A AND B; SETF(NZV); BIT; IR := *PC; PC += 1; END 
void _B0_8_8F_34_0_0(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->b >> 7);
    set_flag(nes, STATUS_FLAG_OVERFLOW, (nes->b >> 6) & 0x01);
    set_flag(nes, STATUS_FLAG_ZERO, (nes->b & nes->acc) == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// A := B; SETF(NZ); IR := *PC; PC += 1; END 
void _A0_8_CC_4_0_0(struct Nes* nes) {
    nes->acc = nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->acc >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->acc == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// X := B; SETF(NZ); IR := *PC; PC += 1; END 
void _30_8_CC_4_0_0(struct Nes* nes) {
    nes->x = nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->x >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->x == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// Y := B; SETF(NZ); IR := *PC; PC += 1; END 
void _40_8_CC_4_0_0(struct Nes* nes) {
    nes->y = nes->b;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->y >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->y == 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// B := *zDP; ML 
void _C0_42_0_0_10_0(struct Nes* nes) {
    nes->b = cpu_bus_read(nes, nes->dpl);
}

// T := 0 LSR B; SETF(NZC) 
void _5B_32_CD_20_0_0_ml6502(struct Nes* nes) {
    uint8_t bit0 = nes->b & 0x01;
    nes->t = nes->b >> 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, 0);
    set_flag(nes, STATUS_FLAG_ZERO, nes->t == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit0);
}

// T := 0 ASL B; SETF(NZC) 
void _5B_32_CD_80_0_0_ml6502(struct Nes* nes) {
    uint8_t bit7 = nes->b >> 7;
    nes->t = nes->b <<= 1;
    set_flag(nes, STATUS_FLAG_NEGATIVE, nes->t >= 0x80);
    set_flag(nes, STATUS_FLAG_ZERO, nes->t == 0);
    set_flag(nes, STATUS_FLAG_CARRY, bit7);
}

// *zDP := T 
void _F5_42_0_0_10_0(struct Nes* nes) {
    cpu_bus_write(nes, nes->dpl, nes->t);
}

// SP += 1; B := *SP 
void _B0_56_0_0_0_0(struct Nes* nes) {
    nes->b = cpu_bus_read(nes, 0x100 | nes->sp);
    nes->sp += 1;
}

// P := *SP; SP += 1 
void _70_56_0_0_10_0(struct Nes* nes) {
    nes->status = cpu_bus_read(nes, 0x100 | nes->sp) & 0b11001111;
    nes->sp += 1;
}

// PCL := *SP; SP += 1 
void _80_56_0_0_10_0(struct Nes* nes) {
    nes->pc &= 0xFF00;
    nes->pc |= cpu_bus_read(nes, 0x100 | nes->sp);
    nes->sp += 1;
}

// PCH := *SP 
void _90_52_0_0_10_0(struct Nes* nes) {
    nes->pc &= 0x00FF;
    nes->pc |= cpu_bus_read(nes, 0x100 | nes->sp) << 8;
}

// *SP := P; SP -= 1 
void _F7_57_0_0_10_0_brk(struct Nes* nes) {
    uint8_t p = nes->status | 0b00110000;
    if (nes->nmi || nes->irq) p &= 0b11101111;
    if (!nes->reset)
        cpu_bus_write(nes, 0x100 | nes->sp, p); //
    nes->sp -= 1;
}

// *SP := P; SP -= 1 
void _F7_57_0_0_10_0_bflag(struct Nes* nes) {
    cpu_bus_write(nes, 0x100 | nes->sp, nes->status | 0b00110000); //
    nes->sp -= 1;
}

// *zDP := X
void _F3_42_0_0_10_0(struct Nes* nes) {
    cpu_bus_write(nes, nes->dpl, nes->x);
}

// *zDP := Y
void _F4_42_0_0_10_0(struct Nes* nes) {
    cpu_bus_write(nes, nes->dpl, nes->y);
}

// *zDP := A 
void _F0_42_0_0_10_0(struct Nes* nes) {
    cpu_bus_write(nes, nes->dpl, nes->acc);
}

// DPL := B + X; B := *zDP
void _3_42_C8_40_0_0(struct Nes* nes) { 
    nes->dpl = nes->b + nes->x;
    nes->b = cpu_bus_read(nes, nes->dpl);
}

// DPL := B + Y; B := *zDP 
void _4_42_C8_40_0_0(struct Nes* nes) {
    nes->dpl = nes->b + nes->y;
    nes->b = cpu_bus_read(nes, nes->dpl);
}

// B := *zDP 
void _B0_42_0_0_10_0(struct Nes* nes) {
    nes->b = cpu_bus_read(nes, nes->dpl);
}

// DPH := *PC; PC += 1 
void _10_0_0_0_0_0(struct Nes* nes) {
    nes->dph = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// B := *DP; ML 
void _C0_12_0_0_10_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    nes->b = cpu_bus_read(nes, dp);
}

// DPL := B + X; DPH.db := *PC; PC += 1 
void _3_80_C8_40_0_0(struct Nes* nes) {
    nes->dpl = nes->b + nes->x;
    nes->ic = nes->b + nes->x > 0xFF;
    nes->dph = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// *DP := T 
void _F5_12_0_0_10_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    cpu_bus_write(nes, dp, nes->t);
}

// *DP := A 
void _F0_12_0_0_10_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    cpu_bus_write(nes, dp, nes->acc);
}

// *DP := X 
void _F3_12_0_0_10_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    cpu_bus_write(nes, dp, nes->x);
}

// *DP := Y 
void _F4_12_0_0_10_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    cpu_bus_write(nes, dp, nes->y);
}

// DPL := B + X; B := *PC; PC += 1 
void _3_0_C8_40_0_0(struct Nes* nes) {
    nes->dpl = nes->b + nes->x;
    nes->ic = nes->b + nes->x > 0xFF;
    nes->b = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// DPL := B + Y; B := *PC; PC += 1 
void _4_0_C8_40_0_0(struct Nes* nes) {
    nes->dpl = nes->b + nes->y;
    nes->ic = nes->b + nes->y > 0xFF;
    nes->b = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// DPH := B + 0; USE(IC); B := *DP 
void _10_12_C8_18_0_0(struct Nes* nes) {
    nes->dph = nes->b + nes->ic;
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    nes->b = cpu_bus_read(nes, dp);
}

// B := *DP 
void _B0_12_0_0_10_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    nes->b = cpu_bus_read(nes, dp);
}

// !! nes->pc = dp !! (how does this micro instruction cause a jump?)
// IR := *DP; PC += 1; END 
void _B0_18_0_4_0_0(struct Nes* nes) {
    uint16_t dp = nes->pc = make_u16(nes->dph, nes->dpl);
    // end
    nes->ir = cpu_bus_read(nes, dp);
    nes->pc += 1;
}

// T := *zDP; DPL += 1 
void _50_44_0_0_10_0(struct Nes* nes) {
    nes->t = cpu_bus_read(nes, nes->dpl);
    nes->dpl += 1;
}

// DPH := *zDP 
void _10_42_0_0_10_0(struct Nes* nes) {
    nes->dph = cpu_bus_read(nes, nes->dpl);
}

// *DPt := A 
void _F0_22_0_0_10_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->t);
    cpu_bus_write(nes, dp, nes->acc);
}

// B := *DPt 
void _B0_22_0_0_10_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->t);
    nes->b = cpu_bus_read(nes, dp);
}

// DPL := B + X; DPH.db := *PC; PC += 1; INCDPH.C 
void _3_80_C8_42_0_0(struct Nes* nes) {
    nes->dpl = nes->b + nes->x;
    nes->dph = cpu_bus_read(nes, nes->pc); // ?
    nes->pc += 1;

    // skip incdph cycle
    uint8_t incdph = nes->b + nes->x > 0xFF;
    if (!incdph) nes->instr_ptr += 1;
}

// DPL := B + Y; DPH.db := *PC; PC += 1; INCDPH.C 
void _4_80_C8_42_0_0(struct Nes* nes) {
    nes->dpl = nes->b + nes->y;
    nes->dph = cpu_bus_read(nes, nes->pc); // ?
    nes->pc += 1;

    // skip incdph cycle
    uint8_t incdph = nes->b + nes->y > 0xFF;
    if (!incdph) nes->instr_ptr += 1;
}

// DPL := B + Y; DPH.db := *zDP; INCDPH.C 
void _4_C2_C8_42_10_0(struct Nes* nes) {
    nes->dph = cpu_bus_read(nes, nes->dpl);
    nes->dpl = nes->b + nes->y;

    // skip incdph cycle
    uint8_t incdph = nes->b + nes->y > 0xFF;
    if (!incdph) nes->instr_ptr += 1;
}

// PCL := *DP; DPL += 1 
void _80_14_0_0_0_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    nes->pc &= 0xFF00;
    nes->pc |= cpu_bus_read(nes, dp);
    nes->dpl += 1;
}

// PCH := *DP 
void _90_12_0_0_0_0(struct Nes* nes) {
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    nes->pc &= 0x00FF;
    nes->pc |= cpu_bus_read(nes, dp) << 8;
}

// *SP := A; SP -= 1 
void _F0_57_0_0_10_0(struct Nes* nes) {
    cpu_bus_write(nes, 0x100 | nes->sp, nes->acc);
    nes->sp -= 1;
}

// B := *zDP; DPL += 1 
void _B0_44_0_0_10_0(struct Nes* nes) {
    nes->b = cpu_bus_read(nes, nes->dpl);
    nes->dpl += 1;
}

// DPL := B + Y; DPH.db := *zDP 
void _4_C2_C8_40_10_0(struct Nes* nes) {
    nes->dph = cpu_bus_read(nes, nes->dpl);
    nes->dpl = nes->b + nes->y;
    nes->ic = nes->b + nes->y > 0xFF;
}

// DPH := DPH + 0; USE(IC); B := *DP 
void _11_12_8_58_0_0(struct Nes* nes) {
    nes->dph += nes->ic;
    uint16_t dp = make_u16(nes->dph, nes->dpl);
    nes->b = cpu_bus_read(nes, dp);
}

// IR := *PC; PC += 1; END 
void _0_8_0_4_0_0(struct Nes* nes) {
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

// B := *SP 
void _B0_52_0_0_10_0(struct Nes* nes) {
    nes->b = cpu_bus_read(nes, 0x100 | nes->sp);
}

// *SP := PCH; SP -= 1 
void _F9_57_0_0_10_0(struct Nes* nes) {
    cpu_bus_write(nes, 0x100 | nes->sp, nes->pc >> 8);
    nes->sp -= 1;
}

// *SP := PCH; SP -= 1 
void _F9_57_0_0_10_0_brk(struct Nes* nes) {
    if (!nes->reset)
        cpu_bus_write(nes, 0x100 | nes->sp, nes->pc >> 8);
    nes->sp -= 1;
}

// *SP := PCL; SP -= 1 
void _F8_57_0_0_10_0(struct Nes* nes) {
    cpu_bus_write(nes, 0x100 | nes->sp, nes->pc & 0xFF);
    nes->sp -= 1;
}

// *SP := PCL; SP -= 1 
void _F8_57_0_0_10_0_brk(struct Nes* nes) {
    if (!nes->reset)
        cpu_bus_write(nes, 0x100 | nes->sp, nes->pc & 0xFF);
    nes->sp -= 1;
}

// *SP := P; SP -= 1; PBR.CLR 
void _F7_57_0_0_10_10(struct Nes* nes) {
   assert(0);
}

// P := *SP 
void _70_52_0_0_10_0(struct Nes* nes) {
    nes->status = cpu_bus_read(nes, 0x0100 | nes->sp) & 0b11001111;
}

// IR := *PC; PC += 1; END.INT 
void _0_8_0_6_0_0(struct Nes* nes) {
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
    nes->reset = nes->nmi = nes->irq = 0;
}

// PCL := PCL + B; EXIT.CC 
void _88_2_C8_41_0_0(struct Nes* nes) {
    uint16_t old_pc = nes->pc;
    uint8_t pcl = nes->pc + nes->b;
    nes->ic = nes->pc + nes->b > 0xFF;
    nes->pc &= 0xFF00;
    nes->pc |= pcl;
    
    //
    uint8_t cont = old_pc >> 8 != (old_pc + (int8_t)nes->b) >> 8; // no way is this right
    if (!cont) nes->instr_ptr += 1;
}

// PCH = PCH + signextend(*); USE(IC) [Carry is always 1?]
void _99_2_C8_78_0_0(struct Nes* nes) {
    uint8_t pch = nes->pc >> 8;
    uint8_t pcl = nes->pc;
    pch += (int8_t)(pcl >= 0x80 ? -1 : 1);
    nes->pc &= 0x00FF;
    nes->pc |= pch << 8;
}

// PCL := *fCP; DPL += 1 
void _80_74_0_0_0_0(struct Nes* nes) {
    uint16_t fcp = 0xFFFE;
    if (nes->reset) fcp = 0xFFFC;
    else if (nes->nmi) fcp = 0xFFFA;
    nes->pc &= 0xFF00;
    nes->pc |= cpu_bus_read(nes, fcp);
    nes->dpl += 1;
}

// PCH := *fDP; SETF(SEI/CLD) 
void _90_62_2_0_0_0(struct Nes* nes) {
    uint16_t fdp = 0xFFFF;
    if (nes->reset) fdp = 0xFFFD;
    else if (nes->nmi) fdp = 0xFFFB;
    nes->pc &= 0x00FF;
    nes->pc |= cpu_bus_read(nes, fdp) << 8;
    set_flag(nes, STATUS_FLAG_INTERRUPT, 1);
    //set_flag(nes, STATUS_FLAG_DECIMAL, 0);
}

// DPL := B := *PC; PC += 1
void _0_0_0_0_0_0(struct Nes* nes) {
    nes->dpl = nes->b = cpu_bus_read(nes, nes->pc); 
    nes->pc += 1;
}

char* opcode_names[] = {
    /*00*/ "brk", /*01*/ "ora ($%04X),x", /*02*/ "unofficial", /*03*/ "unofficial", /*04*/ "unofficial", /*05*/ "ora $%02X", /*06*/ "asl $%02X", /*07*/ "unofficial", /*08*/ "php", /*09*/ "ora #$%02X", /*0A*/ "asl", /*0B*/ "unofficial", /*0C*/ "unofficial", /*0D*/ "ora $%04X", /*0E*/ "asl $%04X", /*0F*/ "unofficial",
    /*10*/ "bpl $%02X", /*11*/ "ora ($%02X,y)", /*12*/ "unofficial", /*13*/ "unofficial", /*14*/ "unofficial", /*15*/ "ora $%02X,x", /*16*/ "asl $%02X,x", /*17*/ "unofficial", /*18*/ "clc", /*19*/ "ora $%04X,y", /*1A*/ "unofficial", /*1B*/ "unofficial", /*1C*/ "unofficial", /*1D*/ "ora $%04X,x", /*1E*/ "asl $%04X,x", /*1F*/ "unofficial",
    /*20*/ "jsr $%04X", /*21*/ "and ($%04X),x", /*22*/ "unofficial", /*23*/ "unofficial", /*24*/ "bit $%02X", /*25*/ "and $%02X", /*26*/ "rol $%02X", /*27*/ "unofficial", /*28*/ "plp", /*29*/ "and #$%02X", /*2A*/ "rol", /*2B*/ "unofficial", /*2C*/ "bit $%04X", /*2D*/ "and $%04X", /*2E*/ "rol $%04X", /*2F*/ "unofficial",
    /*30*/ "bmi $%02X", /*31*/ "and ($%02X,y)", /*32*/ "unofficial", /*33*/ "unofficial", /*34*/ "unofficial", /*35*/ "and $%02X,x", /*36*/ "rol $%02X,x", /*37*/ "unofficial", /*38*/ "sec", /*39*/ "and $%04X,y", /*3A*/ "unofficial", /*3B*/ "unofficial", /*3C*/ "unofficial", /*3D*/ "and $%04X,x", /*3E*/ "rol $%04X,x", /*3F*/ "unofficial",
    /*40*/ "rti", /*41*/ "eor ($%04X),x", /*42*/ "unofficial", /*43*/ "unofficial", /*44*/ "unofficial", /*45*/ "eor $%02X", /*46*/ "lsr $%02X", /*47*/ "unofficial", /*48*/ "pha", /*49*/ "eor #$%02X", /*4A*/ "lsr", /*4B*/ "unofficial", /*4C*/ "jmp $%04X", /*4D*/ "eor $%04X", /*4E*/ "lsr $%04X", /*4F*/ "unofficial",
    /*50*/ "bvc $%02X", /*51*/ "eor ($%02X,y)", /*52*/ "unofficial", /*53*/ "unofficial", /*54*/ "unofficial", /*55*/ "eor $%02X,x", /*56*/ "lsr $%02X,x", /*57*/ "unofficial", /*58*/ "cli", /*59*/ "eor $%04X,y", /*5A*/ "unofficial", /*5B*/ "unofficial", /*5C*/ "unofficial", /*5D*/ "eor $%04X,x", /*5E*/ "lsr $%04X,x", /*5F*/ "unofficial",
    /*60*/ "rts", /*61*/ "adc ($%04X),x", /*62*/ "unofficial", /*63*/ "unofficial", /*64*/ "unofficial", /*65*/ "adc $%02X", /*66*/ "ror $%02X", /*67*/ "unofficial", /*68*/ "pla", /*69*/ "adc #$%02X", /*6A*/ "ror", /*6B*/ "unofficial", /*6C*/ "jmp ($%04X)", /*6D*/ "adc $%04X", /*6E*/ "ror $%04X", /*6F*/ "unofficial",
    /*70*/ "bvs $%02X", /*71*/ "adc ($%02X,y)", /*72*/ "unofficial", /*73*/ "unofficial", /*74*/ "unofficial", /*75*/ "adc $%02X,x", /*76*/ "ror $%02X,x", /*77*/ "unofficial", /*78*/ "sei", /*79*/ "adc $%04X,y", /*7A*/ "unofficial", /*7B*/ "unofficial", /*7C*/ "unofficial", /*7D*/ "adc $%04X,x", /*7E*/ "ror $%04X,x", /*7F*/ "unofficial",
    /*80*/ "unofficial", /*81*/ "sta ($%04X),x", /*82*/ "unofficial", /*83*/ "unofficial", /*84*/ "sty $%02X", /*85*/ "sta $%02X", /*86*/ "stx $%02X", /*87*/ "unofficial", /*88*/ "dey", /*89*/ "unofficial", /*8A*/ "txa", /*8B*/ "unofficial", /*8C*/ "sty $%04X", /*8D*/ "sta $%04X", /*8E*/ "stx $%04X", /*8F*/ "unofficial",
    /*90*/ "bcc $%02X", /*91*/ "sta ($%02X,y)", /*92*/ "unofficial", /*93*/ "unofficial", /*94*/ "sty $%02X,x", /*95*/ "sta $%02X,x", /*96*/ "stx $%02X,y", /*97*/ "unofficial", /*98*/ "tya", /*99*/ "sta $%04X,y", /*9A*/ "txs", /*9B*/ "unofficial", /*9C*/ "unofficial", /*9D*/ "sta $%04X,x", /*9E*/ "unofficial", /*9F*/ "unofficial",
    /*A0*/ "ldy #$%02X", /*A1*/ "lda ($%04X),x", /*A2*/ "ldx #$%02X", /*A3*/ "unofficial", /*A4*/ "ldy $%02X", /*A5*/ "lda $%02X", /*A6*/ "ldx $%02X", /*A7*/ "unofficial", /*A8*/ "tay", /*A9*/ "lda #$%02X", /*AA*/ "tax", /*AB*/ "unofficial", /*AC*/ "ldy $%04X", /*AD*/ "lda $%04X", /*AE*/ "ldx $%04X", /*AF*/ "unofficial",
    /*B0*/ "bcs $%02X", /*B1*/ "lda ($%02X,y)", /*B2*/ "unofficial", /*B3*/ "unofficial", /*B4*/ "ldy $%02X,x", /*B5*/ "lda $%02X,x", /*B6*/ "ldx $%02X,y", /*B7*/ "unofficial", /*B8*/ "clv", /*B9*/ "lda $%04X,y", /*BA*/ "tsx", /*BB*/ "unofficial", /*BC*/ "ldy $%04X,x", /*BD*/ "lda $%04X,x", /*BE*/ "ldx $%04X,y", /*BF*/ "unofficial",
    /*C0*/ "cpy #$%02X", /*C1*/ "cmp ($%04X),x", /*C2*/ "unofficial", /*C3*/ "unofficial", /*C4*/ "cpy $%02X", /*C5*/ "cmp $%02X", /*C6*/ "dec $%02X", /*C7*/ "unofficial", /*C8*/ "iny", /*C9*/ "cmp #$%02X", /*CA*/ "dex", /*CB*/ "unofficial", /*CC*/ "cpy $%04X", /*CD*/ "cmp $%04X", /*CE*/ "dec $%04X", /*CF*/ "unofficial",
    /*D0*/ "bne $%02X", /*D1*/ "cmp ($%02X,y)", /*D2*/ "unofficial", /*D3*/ "unofficial", /*D4*/ "unofficial", /*D5*/ "cmp $%02X,x", /*D6*/ "dec $%02X,x", /*D7*/ "unofficial", /*D8*/ "cld", /*D9*/ "cmp $%04X,y", /*DA*/ "unofficial", /*DB*/ "unofficial", /*DC*/ "unofficial", /*DD*/ "cmp $%04X,x", /*DE*/ "dec $%04X,x", /*DF*/ "unofficial",
    /*E0*/ "cpx #$%02X", /*E1*/ "sbc ($%04X),x", /*E2*/ "unofficial", /*E3*/ "unofficial", /*E4*/ "cpx $%02X", /*E5*/ "sbc $%02X", /*E6*/ "inc $%02X", /*E7*/ "unofficial", /*E8*/ "inx", /*E9*/ "sbc #$%02X", /*EA*/ "nop", /*EB*/ "unofficial", /*EC*/ "cpx $%04X", /*ED*/ "sbc $%04X", /*EE*/ "inc $%04X", /*EF*/ "unofficial",
    /*F0*/ "beq $%02X", /*F1*/ "sbc ($%02X,y)", /*F2*/ "unofficial", /*F3*/ "unofficial", /*F4*/ "unofficial", /*F5*/ "sbc $%02X,x", /*F6*/ "inc $%02X,x", /*F7*/ "unofficial", /*F8*/ "sed", /*F9*/ "sbc $%04X,y", /*FA*/ "unofficial", /*FB*/ "unofficial", /*FC*/ "unofficial", /*FD*/ "sbc $%04X,x", /*FE*/ "inc $%04X,x", /*FF*/ "unofficial",
};
uint8_t opcode_sizes[] = {
    /*00*/ 1, /*01*/ 2, /*02*/ 9, /*03*/ 9, /*04*/ 9, /*05*/ 1, /*06*/ 1, /*07*/ 9, /*08*/ 0, /*09*/ 1, /*0A*/ 0, /*0B*/ 9, /*0C*/ 9, /*0D*/ 2, /*0E*/ 2, /*0F*/ 9,
    /*10*/ 1, /*11*/ 1, /*12*/ 9, /*13*/ 9, /*14*/ 9, /*15*/ 1, /*16*/ 1, /*17*/ 9, /*18*/ 0, /*19*/ 2, /*1A*/ 9, /*1B*/ 9, /*1C*/ 9, /*1D*/ 2, /*1E*/ 2, /*1F*/ 9,
    /*20*/ 0, /*21*/ 2, /*22*/ 9, /*23*/ 9, /*24*/ 1, /*25*/ 1, /*26*/ 1, /*27*/ 9, /*28*/ 0, /*29*/ 1, /*2A*/ 0, /*2B*/ 9, /*2C*/ 2, /*2D*/ 2, /*2E*/ 2, /*2F*/ 9,
    /*30*/ 1, /*31*/ 1, /*32*/ 9, /*33*/ 9, /*34*/ 9, /*35*/ 1, /*36*/ 1, /*37*/ 9, /*38*/ 0, /*39*/ 2, /*3A*/ 9, /*3B*/ 9, /*3C*/ 9, /*3D*/ 2, /*3E*/ 2, /*3F*/ 9,
    /*40*/ 0, /*41*/ 2, /*42*/ 9, /*43*/ 9, /*44*/ 9, /*45*/ 1, /*46*/ 1, /*47*/ 9, /*48*/ 0, /*49*/ 1, /*4A*/ 0, /*4B*/ 9, /*4C*/ 2, /*4D*/ 2, /*4E*/ 2, /*4F*/ 9,
    /*50*/ 1, /*51*/ 1, /*52*/ 9, /*53*/ 9, /*54*/ 9, /*55*/ 1, /*56*/ 1, /*57*/ 9, /*58*/ 0, /*59*/ 2, /*5A*/ 9, /*5B*/ 9, /*5C*/ 9, /*5D*/ 2, /*5E*/ 2, /*5F*/ 9,
    /*60*/ 0, /*61*/ 2, /*62*/ 9, /*63*/ 9, /*64*/ 9, /*65*/ 1, /*66*/ 1, /*67*/ 9, /*68*/ 0, /*69*/ 1, /*6A*/ 0, /*6B*/ 9, /*6C*/ 2, /*6D*/ 2, /*6E*/ 2, /*6F*/ 9,
    /*70*/ 1, /*71*/ 1, /*72*/ 9, /*73*/ 9, /*74*/ 9, /*75*/ 1, /*76*/ 1, /*77*/ 9, /*78*/ 0, /*79*/ 2, /*7A*/ 9, /*7B*/ 9, /*7C*/ 9, /*7D*/ 2, /*7E*/ 2, /*7F*/ 9,
    /*80*/ 9, /*81*/ 2, /*82*/ 9, /*83*/ 9, /*84*/ 1, /*85*/ 1, /*86*/ 1, /*87*/ 9, /*88*/ 0, /*89*/ 9, /*8A*/ 0, /*8B*/ 9, /*8C*/ 2, /*8D*/ 2, /*8E*/ 2, /*8F*/ 9,
    /*90*/ 1, /*91*/ 1, /*92*/ 9, /*93*/ 9, /*94*/ 1, /*95*/ 1, /*96*/ 1, /*97*/ 9, /*98*/ 0, /*99*/ 2, /*9A*/ 0, /*9B*/ 9, /*9C*/ 9, /*9D*/ 2, /*9E*/ 9, /*9F*/ 9,
    /*A0*/ 1, /*A1*/ 2, /*A2*/ 1, /*A3*/ 9, /*A4*/ 1, /*A5*/ 1, /*A6*/ 1, /*A7*/ 9, /*A8*/ 0, /*A9*/ 1, /*AA*/ 0, /*AB*/ 9, /*AC*/ 2, /*AD*/ 2, /*AE*/ 2, /*AF*/ 9,
    /*B0*/ 1, /*B1*/ 1, /*B2*/ 9, /*B3*/ 9, /*B4*/ 1, /*B5*/ 1, /*B6*/ 1, /*B7*/ 9, /*B8*/ 0, /*B9*/ 2, /*BA*/ 0, /*BB*/ 9, /*BC*/ 2, /*BD*/ 2, /*BE*/ 2, /*BF*/ 9,
    /*C0*/ 1, /*C1*/ 2, /*C2*/ 9, /*C3*/ 9, /*C4*/ 1, /*C5*/ 1, /*C6*/ 1, /*C7*/ 9, /*C8*/ 0, /*C9*/ 1, /*CA*/ 0, /*CB*/ 9, /*CC*/ 2, /*CD*/ 2, /*CE*/ 2, /*CF*/ 9,
    /*D0*/ 1, /*D1*/ 1, /*D2*/ 9, /*D3*/ 9, /*D4*/ 9, /*D5*/ 1, /*D6*/ 1, /*D7*/ 9, /*D8*/ 0, /*D9*/ 2, /*DA*/ 9, /*DB*/ 9, /*DC*/ 9, /*DD*/ 2, /*DE*/ 2, /*DF*/ 9,
    /*E0*/ 1, /*E1*/ 2, /*E2*/ 9, /*E3*/ 9, /*E4*/ 1, /*E5*/ 1, /*E6*/ 1, /*E7*/ 9, /*E8*/ 0, /*E9*/ 1, /*EA*/ 0, /*EB*/ 9, /*EC*/ 2, /*ED*/ 2, /*EE*/ 2, /*EF*/ 9,
    /*F0*/ 1, /*F1*/ 1, /*F2*/ 9, /*F3*/ 9, /*F4*/ 9, /*F5*/ 1, /*F6*/ 1, /*F7*/ 9, /*F8*/ 0, /*F9*/ 2, /*FA*/ 9, /*FB*/ 9, /*FC*/ 9, /*FD*/ 2, /*FE*/ 2, /*FF*/ 9,
};

// Special version of _0_0_0_0_0_0, used after the previous opcode is END
void _0_0_0_0_0_0_end(struct Nes* nes) { 
    nes->dpl = nes->b = cpu_bus_read(nes, nes->pc); 

    uint16_t s = nes->b;
    if (opcode_sizes[nes->ir] == 2) {
        s |= cpu_bus_read(nes, nes->pc + 1) << 8;
    }
    char* c = opcode_names[nes->ir];
    //nlog(c, s);

    // if an interrupt is raised, force next instruction to BRK
    nes->irq = nes->cartridge->irq;
    nes->cartridge->irq = 0;
    if (nes->reset || nes->nmi || (nes->irq && !get_flag(nes, STATUS_FLAG_INTERRUPT))) {
        // priority: reset > nmi > irq
        if (nes->reset) nes->nmi = nes->irq = 0;
        if (nes->nmi) nes->irq = 0;

        nes->pc -= 2; // undo prev fetch opcode inc
        nes->ir = 0x00; // force brk
    }

    // If the next opcode is single byte, don't increment PC
    switch (nes->ir) {
        /*LSR*/ case 0x4A:
        /*ASL*/ case 0x0A:
        /*ROR*/ case 0x6A:
        /*ROL*/ case 0x2A:
        /*INX*/ case 0xE8:
        /*DEX*/ case 0xCA:
        /*TAX*/ case 0xAA:
        /*TXA*/ case 0x8A:
        /*TXS*/ case 0x9A:
        /*TSX*/ case 0xBA:
        /*INY*/ case 0xC8:
        /*DEY*/ case 0x88:
        /*TAY*/ case 0xA8:
        /*TYA*/ case 0x98:
        /*CLC*/ case 0x18:
        /*SEC*/ case 0x38:
        /*CLI*/ case 0x58:
        /*SEI*/ case 0x78:
        /*CLV*/ case 0xB8:
        /*CLD*/ case 0xD8:
        /*SED*/ case 0xF8:
        /*PHP*/ case 0x08:
        /*PLP*/ case 0x28:
        /*PHA*/ case 0x48:
        /*PLA*/ case 0x68:
        /*RTS*/ case 0x60:
        /*RTI*/ case 0x40: 
        /*NOP*/ case 0xEA: break;
        default: nes->pc += 1; break;
    }

    // Jump to next micro instruction
    nes->instr_ptr = lookup[nes->ir];
}

// DPL := B := *PC
void _0_0_0_0_0_0_noinc(struct Nes* nes) { 
    nes->dpl = nes->b = cpu_bus_read(nes, nes->pc); 
}

void _B0_8_F9_4_0_0_carry(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_CARRY, 1);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

void _B0_8_F9_4_0_0_interrupt(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_INTERRUPT, 1);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

void _B0_8_F9_4_0_0_decimal(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_DECIMAL, 1);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

void _B0_8_9_4_0_0_carry(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_CARRY, 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

void _B0_8_9_4_0_0_interrupt(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_INTERRUPT, 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

void _B0_8_9_4_0_0_overflow(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_OVERFLOW, 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;
}

void _B0_8_9_4_0_0_decimal(struct Nes* nes) {
    set_flag(nes, STATUS_FLAG_DECIMAL, 0);
    // end
    nes->ir = cpu_bus_read(nes, nes->pc);
    nes->pc += 1;    
}

// DPH := DPH + 1
void _INCDPH(struct Nes* nes) {
    nes->dph += 1;
}

void _bpl(struct Nes* nes) {
    if (!get_flag(nes, STATUS_FLAG_NEGATIVE)) {
        _88_2_C8_41_0_0(nes);
    } else {
        _0_8_0_4_0_0(nes);
        nes->instr_ptr += 2;
    }
}

void _bmi(struct Nes* nes) {
    if (get_flag(nes, STATUS_FLAG_NEGATIVE)) {
        _88_2_C8_41_0_0(nes);
    } else {
        _0_8_0_4_0_0(nes);
        nes->instr_ptr += 2;
    }
}

void _bvc(struct Nes* nes) {
    if (!get_flag(nes, STATUS_FLAG_OVERFLOW)) {
        _88_2_C8_41_0_0(nes);
    } else {
        _0_8_0_4_0_0(nes);
        nes->instr_ptr += 2;
    }
}

void _bvs(struct Nes* nes) {
    if (get_flag(nes, STATUS_FLAG_OVERFLOW)) {
        _88_2_C8_41_0_0(nes);
    } else {
        _0_8_0_4_0_0(nes);
        nes->instr_ptr += 2;
    }
}

void _bcc(struct Nes* nes) {
    if (!get_flag(nes, STATUS_FLAG_CARRY)) {
        _88_2_C8_41_0_0(nes);
    } else {
        _0_8_0_4_0_0(nes);
        nes->instr_ptr += 2;
    }
}

void _bcs(struct Nes* nes) {
    if (get_flag(nes, STATUS_FLAG_CARRY)) {
        _88_2_C8_41_0_0(nes);
    } else {
        _0_8_0_4_0_0(nes);
        nes->instr_ptr += 2;
    }
}

void _bne(struct Nes* nes) {
    if (!get_flag(nes, STATUS_FLAG_ZERO)) {
        _88_2_C8_41_0_0(nes);
    } else {
        _0_8_0_4_0_0(nes);
        nes->instr_ptr += 2;
    }
}

void _beq(struct Nes* nes) {
    if (get_flag(nes, STATUS_FLAG_ZERO)) {
        _88_2_C8_41_0_0(nes);
    } else {
        _0_8_0_4_0_0(nes);
        nes->instr_ptr += 2;
    }
}