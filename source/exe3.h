typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

void hook_sub3007474(u32 * regs);
void hook_sub8035fc8(u32 * regs);
void hook_sub8034058(u32 * regs);
u32* restoreRegs(u32 * dest,u32 * src);
u32* copyRegs(u32 * dest,u32 * src);