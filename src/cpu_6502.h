#ifndef __CPU_6502_H__
#define __CPU_6502_H__

#include <cstdint>

typedef unsigned char bit;

class memory_map {
private:
	const size_t memsize = 0x10000;
	uint8_t *mem;

public:
	memory_map() : mem(new uint8_t[memsize]) {}
	~memory_map() { delete[] mem; }

	inline uint8_t readByte(uint16_t addr) const {
		addr %= memsize;
		return mem[addr];
	}

	inline uint16_t readWord(uint16_t addr) const {
		addr %= memsize;
		return *((uint16_t *)&mem[addr]);
	}

	inline uint8_t writeByte(uint16_t addr, uint8_t byteData) {
		addr %= memsize;
		return mem[addr] = byteData;
	}

	inline uint16_t writeWord(uint16_t addr, uint16_t wordData) {
		addr %= memsize;
		return *((uint16_t *)&mem[addr]) = wordData;
	}
};

class cpu_6502_register {
private:
	uint16_t __PC;
	uint8_t __S;
	uint8_t __X;
	uint8_t __Y;
	uint8_t __A;
	//uint8_t __P;
	bit __flagC;
	bit __flagZ;
	bit __flagI;
	bit __flagD;
	bit __flagB;
	bit __flagV;
	bit __flagN;

/*
Bits Name  Expl.
8    A     Accumulator
8    X     Index Register X
8    Y     Index Register Y
16   PC    Program Counter
8    S     Stack Pointer (see below)
8    P     Processor Status Register (see below)
*/

/*
Bit  Name  Expl.
0    C     Carry         (0=No Carry, 1=Carry)
1    Z     Zero          (0=Nonzero, 1=Zero)
2    I     IRQ Disable   (0=IRQ Enable, 1=IRQ Disable)
3    D     Decimal Mode  (0=Normal, 1=BCD Mode for ADC/SBC opcodes)
4    B     Break Flag    (0=IRQ/NMI, 1=RESET or BRK/PHP opcode)
5    -     Not used      (Always 1)
6    V     Overflow      (0=No Overflow, 1=Overflow)
7    N     Negative/Sign (0=Positive, 1=Negative)
*/

public:

	cpu_6502_register() {}
	~cpu_6502_register() {}

	template <typename T>
	inline bool isNegative(T res) {
		return ((res >> (sizeof(T) - 1)) & 0x0001UL) ? true : false;
	}

	template <typename T>
	inline bool isZero(T res) {
		return res == 0 ? true : false;
	}

	inline uint8_t setX(uint8_t i) { return __X = i; }
	inline uint8_t setY(uint8_t i) { return __Y = i; }
	inline uint8_t setA(uint8_t i) { return __A = i; }
	inline uint8_t setS(uint8_t i) { return __S = i; }
	inline uint8_t setPC(uint16_t i) { return __PC = i; }
	inline uint8_t getX() const { return __X; }
	inline uint8_t getY() const { return __Y; }
	inline uint8_t getA() const { return __A; }
	inline uint8_t getS() const { return __S; }
	inline uint16_t getPC() const { return __PC; }

	template <typename T>
	inline void checkFlagN(T res) {
		setFlagN(isNegative(res) ? 1 : 0);
	}

	template <typename T>
	inline void checkFlagZ(T res) {
		setFlagZ(isZero(res) ? 1 : 0);
	}

	inline bit setFlagN(bit flag) { return __flagN = flag; }
	inline bit setFlagV(bit flag) { return __flagV = flag; }
	inline bit setFlagB(bit flag) { return __flagB = flag; }
	inline bit setFlagD(bit flag) { return __flagD = flag; }
	inline bit setFlagI(bit flag) { return __flagI = flag; }
	inline bit setFlagZ(bit flag) { return __flagZ = flag; }
	inline bit setFlagC(bit flag) { return __flagC = flag; }

	inline bit getFlagN() const { return __flagN; }
	inline bit getFlagV() const { return __flagV; }
	inline bit getFlagB() const { return __flagB; }
	inline bit getFlagD() const { return __flagD; }
	inline bit getFlagI() const { return __flagI; }
	inline bit getFlagZ() const { return __flagZ; }
	inline bit getFlagC() const { return __flagC; }

};

class cpu_6502 {
private:
	cpu_6502_register __reg;
	memory_map __memmap;

public:
	cpu_6502(cpu_6502_register& reg, memory_map& memmap) : __reg(reg), __memmap(memmap) {}
	~cpu_6502() {}
	cpu_6502_register& getReg() { return __reg; }
	memory_map& getMemMap() { return __memmap; }

	// Register/Immeditate to Register Transfer

	// MOV Y, A
	inline void __handle_opcode_A8() {
		auto res = __reg.setY(__reg.getA());
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV X, A
	inline void __handle_opcode_AA() {
		auto res = __reg.setX(__reg.getA());
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV X, S
	inline void __handle_opcode_BA() {
		auto res = __reg.setX(__reg.getS());
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV A, Y
	inline void __handle_opcode_98() {
		auto res = __reg.setA(__reg.getY());
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV A,X
	inline void __handle_opcode_8A() {
		auto res = __reg.setA(__reg.getX());
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV S, X
	inline void __handle_opcode_9A() {
		auto res = __reg.setS(__reg.getX());
	}

	// MOV A,nn
	inline void __handle_opcode_A9nn(uint8_t nn) {
		auto res = __reg.setA(nn);
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV X, nn
	inline void __handle_opcode_A2nn(uint8_t nn) {
		auto res = __reg.setX(nn);
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV Y, nn
	inline void __handle_opcode_A0nn(uint8_t nn) {
		auto res = __reg.setY(nn);
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// Load Register from Memory

	// MOV A, [nn]
	inline void __handle_opcode_A5nn(uint8_t nn) {
		auto res = __reg.setA(__memmap.readByte(nn));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV A, [nn + X]
	inline void __handle_opcode_B5nn(uint8_t nn) {
		auto res = __reg.setA(__memmap.readByte(nn + __reg.getX()));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV A, [nnnn]
	inline void __handle_opcode_ADnnnn(uint16_t nnnn) {
		auto res = __reg.setA(__memmap.readByte(nnnn));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV A, [nnnn + X]
	inline void __handle_opcode_BDnnnn(uint16_t nnnn) {
		auto res = __reg.setA(__memmap.readByte(nnnn + __reg.getX()));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV A, [nnnn + Y]
	inline void __handle_opcode_B9nnnn(uint16_t nnnn) {
		auto res = __reg.setA(__memmap.readByte(nnnn + __reg.getY()));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV A, [[nn + X]]
	inline void __handle_opcode_A1nn(uint8_t nn) {
		auto res = __reg.setA(__memmap.readByte(__memmap.readByte(nn + __reg.getX())));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV A, [[nn] + Y]
	inline void __handle_opcode_B1nn(uint8_t nn) {
		auto res = __reg.setA(__memmap.readByte(__memmap.readByte(nn) + __reg.getY()));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV X, [nn]
	inline void __handle_opcode_A6nn(uint8_t nn) {
		auto res = __reg.setX(__memmap.readByte(nn));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV X, [nn + Y]
	inline void __handle_opcode_B6nn(uint8_t nn) {
		auto res = __reg.setX(__memmap.readByte(nn + __reg.getY()));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV X, [nnnn]
	inline void __handle_opcode_AEnnnn(uint16_t nnnn) {
		auto res = __reg.setX(__memmap.readByte(nnnn));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV X, [nnnn + Y]
	inline void __handle_opcode_BEnnnn(uint16_t nnnn) {
		auto res = __reg.setX(__memmap.readByte(nnnn + __reg.getY()));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV Y, [nn]
	inline void __handle_opcode_A4nn(uint8_t nn) {
		auto res = __reg.setY(__memmap.readByte(nn));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV Y, [nn + X]
	inline void __handle_opcode_B4nn(uint8_t nn) {
		auto res = __reg.setY(__memmap.readByte(nn + __reg.getX()));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV Y, [nnnn]
	inline void __handle_opcode_ACnnnn(uint16_t nnnn) {
		auto res = __reg.setY(__memmap.readByte(nnnn));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// MOV Y, [nnnn + X]
	inline void __handle_opcode_BCnnnn(uint16_t nnnn) {
		auto res = __reg.setY(__memmap.readByte(nnnn + __reg.getX()));
		__reg.checkFlagN(res);
		__reg.checkFlagZ(res);
	}

	// Store Register in Memory

	// MOV [nn], A
	inline void __handle_opcode_85nn(uint8_t nn) {
		__memmap.writeByte(nn, __reg.getA());
	}

	// MOV [nn + X], A
	inline void __handle_opcode_95nn(uint8_t nn) {
		__memmap.writeByte(nn + __reg.getX(), __reg.getA());
	}

	// MOV [nnnn], A
	inline void __handle_opcode_8Dnnnn(uint16_t nnnn) {
		__memmap.writeByte(nnnn, __reg.getA());
	}

	// MOV [nnnn + X], A
	inline void __handle_opcode_9Dnnnn(uint16_t nnnn) {
		__memmap.writeByte(nnnn + __reg.getX(), __reg.getA());
	}

	// MOV [nnnn + Y], A
	inline void __handle_opcode_99nnnn(uint16_t nnnn) {
		__memmap.writeByte(nnnn + __reg.getY(), __reg.getA());
	}

	// MOV [[nn + X]], A
	inline void __handle_opcode_81nn(uint8_t nn) {
		__memmap.writeByte(__memmap.readByte(nn + __reg.getX()), __reg.getA());
	}

	// MOV [[nn] + Y], A
	inline void __handle_opcode_91nn(uint8_t nn) {
		__memmap.writeByte(__memmap.readByte(nn + __reg.getX()), __reg.getA());
	}

	inline void __handle_opcode_86nn(uint8_t nn) {

	}

};




#endif // __CPU_6502_H__
