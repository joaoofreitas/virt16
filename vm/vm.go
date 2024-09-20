package main

type VM struct {
   // Memory 32 768 words of 16 bits each
   Memory [32768]uint16

   // Registers 16 bit * 15 GPR
   Registers [15]uint16  // General Purpose Registers
   Peripherals [4]uint16  // Peripherals
   Video uint16 	// Contains the Address of the video memory
   Accumulator uint16  // Accumulator
   Time uint16  // Time
   
   PC uint16  // Program Counter
   SP uint16  // Stack Pointer

    // Flags
   Z bool   
   G bool 
   L bool
   E bool
}

// Constructor
func NewVM() *VM {
    vm := &VM{}
    return vm
}

func (vm *VM) reset() {
    // Not implemented
}

func (vm *VM) loadProgram(program []uint16) {
    // Not implemented 
}

func (vm *VM) execute() {
    // Not implemented
}
