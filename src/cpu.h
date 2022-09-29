// Steps the CPU one cycle.
void step_cpu(struct Nes*);

// Functions emulating the CPU bus (https://wiki.nesdev.org/w/index.php/CPU_memory_map).
uint8_t cpu_bus_read(struct Nes*, uint16_t);
void cpu_bus_write(struct Nes*, uint16_t, uint8_t);

// Flag setters/getters.
void set_flag(struct Nes*, uint8_t, uint8_t);
uint8_t get_flag(struct Nes*, uint8_t);

// microcode
typedef void(micro_instr)(struct Nes* nes);
micro_instr** get_micro(uint8_t);
void _A0_8_AD_24_0_0(struct Nes*);
void _A0_8_AD_44_0_0(struct Nes*);
void _A0_8_AD_2C_0_0(struct Nes*);
void _5B_32_CD_28_0_0_ml6502(struct Nes*);
void _A0_8_AD_54_0_0(struct Nes*);
void _5B_32_CD_90_0_0_ml6502(struct Nes*);
void _50_32_C_88_0_0_ml6502(struct Nes*);
void _50_32_FC_80_0_0_ml6502(struct Nes*);
void _A0_8_CE_55_0_0(struct Nes*);
void _A0_8_3E_55_0_0(struct Nes*);
void _33_8_C_4C_0_0(struct Nes*);
void _33_8_FC_44_0_0(struct Nes*);
void _30_42_0_0_10_0(struct Nes*);
void _A3_8_C_44_0_0(struct Nes*);
void _23_8_8_44_0_0(struct Nes*);
void _32_8_C_44_0_0(struct Nes*);
void _44_8_C_4C_0_0(struct Nes*);
void _44_8_FC_44_0_0(struct Nes*);
void _40_8_C_44_0_0(struct Nes*);
void _A4_8_C_44_0_0(struct Nes*);
void _A0_8_6C_4_0_0(struct Nes*);
void _A0_8_EC_4_0_0(struct Nes*);
void _B0_8_3D_4C_0_0(struct Nes*);
void _B3_8_3D_4C_0_0(struct Nes*);
void _B4_8_3D_4C_0_0(struct Nes*);
void _A0_8_8C_4_0_0(struct Nes*);
void _B0_8_8F_34_0_0(struct Nes*);
void _A0_8_CC_4_0_0(struct Nes*);
void _30_8_CC_4_0_0(struct Nes*);
void _40_8_CC_4_0_0(struct Nes*);
void _C0_42_0_0_10_0(struct Nes*);
void _5B_32_CD_20_0_0_ml6502(struct Nes*);
void _5B_32_CD_80_0_0_ml6502(struct Nes*);
void _F5_42_0_0_10_0(struct Nes*);
void _B0_56_0_0_0_0(struct Nes*);
void _70_56_0_0_10_0(struct Nes*);
void _80_56_0_0_10_0(struct Nes*);
void _90_52_0_0_10_0(struct Nes*);
void _F7_57_0_0_10_0(struct Nes*);
void _F3_42_0_0_10_0(struct Nes*);
void _F4_42_0_0_10_0(struct Nes*);
void _F0_42_0_0_10_0(struct Nes*);
void _3_42_C8_40_0_0(struct Nes*);
void _4_42_C8_40_0_0(struct Nes*);
void _B0_42_0_0_10_0(struct Nes*);
void _10_0_0_0_0_0(struct Nes*);
void _C0_12_0_0_10_0(struct Nes*);
void _3_80_C8_40_0_0(struct Nes*);
void _F5_12_0_0_10_0(struct Nes*);
void _F0_12_0_0_10_0(struct Nes*);
void _F3_12_0_0_10_0(struct Nes*);
void _F4_12_0_0_10_0(struct Nes*);
void _3_0_C8_40_0_0(struct Nes*);
void _4_0_C8_40_0_0(struct Nes*);
void _10_12_C8_18_0_0(struct Nes*);
void _B0_12_0_0_10_0(struct Nes*);
void _B0_18_0_4_0_0(struct Nes*);
void _50_44_0_0_10_0(struct Nes*);
void _10_42_0_0_10_0(struct Nes*);
void _F0_22_0_0_10_0(struct Nes*);
void _B0_22_0_0_10_0(struct Nes*);
void _3_80_C8_42_0_0(struct Nes*);
void _4_80_C8_42_0_0(struct Nes*);
void _4_C2_C8_42_10_0(struct Nes*);
void _80_14_0_0_0_0(struct Nes*);
void _90_12_0_0_0_0(struct Nes*);
void _F0_57_0_0_10_0(struct Nes*);
void _B0_44_0_0_10_0(struct Nes*);
void _4_C2_C8_40_10_0(struct Nes*);
void _11_12_8_58_0_0(struct Nes*);
void _0_8_0_4_0_0(struct Nes*);
void _B0_52_0_0_10_0(struct Nes*);
void _F9_57_0_0_10_0(struct Nes*);
void _F8_57_0_0_10_0(struct Nes*);
void _F7_57_0_0_10_10(struct Nes*);
void _70_52_0_0_10_0(struct Nes*);
void _0_8_0_6_0_0(struct Nes*);
void _88_2_C8_41_0_0(struct Nes*);
void _99_2_C8_78_0_0(struct Nes*);
void _F9_57_0_0_10_0_brk(struct Nes*);
void _F8_57_0_0_10_0_brk(struct Nes*);
void _F7_57_0_0_10_0_brk(struct Nes*);
void _80_74_0_0_0_0(struct Nes*);
void _90_62_2_0_0_0(struct Nes*);
void _0_0_0_0_0_0(struct Nes*);
void _F7_57_0_0_10_0_bflag(struct Nes*);
void _0_0_0_0_0_0_end(struct Nes*);
void _B0_8_9_4_0_0_carry(struct Nes*);
void _B0_8_F9_4_0_0_carry(struct Nes*);
void _B0_8_9_4_0_0_interrupt(struct Nes*);
void _B0_8_F9_4_0_0_interrupt(struct Nes*);
void _B0_8_9_4_0_0_overflow(struct Nes*);
void _B0_8_9_4_0_0_decimal(struct Nes*);
void _B0_8_F9_4_0_0_decimal(struct Nes*);
void _INCDPH(struct Nes*);
void _bpl(struct Nes*); // awful hack
void _bmi(struct Nes*); // awful hack
void _bvc(struct Nes*); // awful hack
void _bvs(struct Nes*); // awful hack
void _bcc(struct Nes*); // awful hack
void _bcs(struct Nes*); // awful hack
void _bne(struct Nes*); // awful hack
void _beq(struct Nes*); // awful hack