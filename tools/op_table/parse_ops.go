package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"strconv"
)

// TableFlags : stuff
type TableFlags struct {
	N string `json:"N"`
	Z string `json:"Z"`
	C string `json:"C"`
	I string `json:"I"`
	D string `json:"D"`
	V string `json:"V"`
}

// Table : stuff
type Table struct {
	Name                  string     `json:"Name"`
	AddressingMode        int        `json:"AddressingMode"`
	Opcode                int        `json:"Opcode"`
	Length                int        `json:"Length"`
	Cycles                int        `json:"Cycles"`
	ExtraCycleOnBranch    bool       `json:"ExtraCycleOnBranch"`
	ExtraCycleOnPageCross bool       `json:"ExtraCycleOnPageCross"`
	Illegal               bool       `json:"Illegal"`
	Flags                 TableFlags `json:"Flags"`
	Summary               string     `json:"Summary"`
}

const (
	IMPLIED     = 0
	ACCUMULATOR = 1
	RELATIVE    = 2
	IMMEDIATE   = 3
	ABSOLUTE    = 4
	ABSOLUTEX   = 5
	ABSOLUTEY   = 6
	ZEROPAGE    = 7
	ZEROPAGEX   = 8
	ZEROPAGEY   = 9
	INDIRECT    = 10
	INDIRECTX   = 11
	INDIRECTY   = 12
	// HEADER : app header
	HEADER = "/*parse_ops - 0.0.1: auto generate C/C++ array from table data*/\n\n"
	// INFILE : table data to parse
	INFILE = "table.json"
	// OUTFILE : file output
	OUTFILE = "cycle_table.h"
	// DEBUGOUTFILE : file output
	DEBUGOUTFILE = "opcode_info_table.h"
)

func int2Hex(a int) string {
	return fmt.Sprintf("0x%02X", a)
}

func bool2StrInt(a bool) string {
	if a {
		return strconv.Itoa(1)
	}
	return strconv.Itoa(0)
}

func dump(table []Table) {
	file, err := os.Create(OUTFILE)
	if err != nil {
		log.Fatal(err)
	}

	// header + include guards
	file.WriteString(HEADER + "#ifndef CYCLE_TABLE_H\n#define CYCLE_TABLE_H\n")

	// write out struct
	file.WriteString("\nstruct CyclePair {\n")
	file.WriteString("\tunsigned char c;\n")
	file.WriteString("\tunsigned char p;\n")
	file.WriteString("};\n")

	// write out CyclePair array
	file.WriteString("\nstatic const struct CyclePair CYCLE_PAIR_TABLE[0x100] = {\n\t")
	for i, data := range table {
		if (i%0x10) == 0 && i != 0 {
			file.WriteString("\n\t")
		}
		file.WriteString("{" + strconv.Itoa(data.Cycles) + "," + bool2StrInt(data.ExtraCycleOnPageCross) + "},")
	}
	file.WriteString("\n};\n")

	// end include guard
	file.WriteString("\n#endif /* CYCLE_TABLE_H */\n")
}

func dumpDebug(table []Table) {
	// DEBUG FILE
	file, err := os.Create(DEBUGOUTFILE)
	if err != nil {
		log.Fatal(err)
	}

	// header + include guards
	file.WriteString(HEADER + "#ifndef OPCODE_INFO_TABLE_H\n#define OPCODE_INFO_TABLE_H\n")

	// write out enum
	file.WriteString("\nenum NesAdressingMode {\n")
	file.WriteString("\tADDRESSING_MODE_IMPLIED,\n")
	file.WriteString("\tADDRESSING_MODE_ACCUMULATOR,\n")
	file.WriteString("\tADDRESSING_MODE_RELATIVE,\n")
	file.WriteString("\tADDRESSING_MODE_IMMEDIATE,\n")
	file.WriteString("\tADDRESSING_MODE_ABSOLUTE,\n")
	file.WriteString("\tADDRESSING_MODE_ABSOLUTE_X,\n")
	file.WriteString("\tADDRESSING_MODE_ABSOLUTE_Y,\n")
	file.WriteString("\tADDRESSING_MODE_ZEROPAGE,\n")
	file.WriteString("\tADDRESSING_MODE_ZEROPAGE_X,\n")
	file.WriteString("\tADDRESSING_MODE_ZEROPAGE_Y,\n")
	file.WriteString("\tADDRESSING_MODE_INDIRECT,\n")
	file.WriteString("\tADDRESSING_MODE_INDIRECT_X,\n")
	file.WriteString("\tADDRESSING_MODE_INDIRECT_Y\n")
	file.WriteString("};\n")

	// write out struct
	file.WriteString("\nstruct OpcodeInfo {\n")
	file.WriteString("\tconst char* const name; /* name of the instruction. */\n")
	file.WriteString("\tconst char* const flags; /* 0 = unset, 1 = set */\n")
	file.WriteString("\tconst char* const summary; /* short description of the instruction. */\n")
	file.WriteString("\tconst unsigned char opcode; /* opcode hex value. */\n")
	file.WriteString("\tconst unsigned char addressing_mode; /* see enum NesAdressingMode. */\n")
	file.WriteString("\tconst unsigned char length; /*  */\n")
	file.WriteString("\tconst unsigned char cycles; /* how many cycles the instruction takes. */\n")
	file.WriteString("\tconst unsigned char extra_cycle_on_branch; /* boolean. */\n")
	file.WriteString("\tconst unsigned char extra_cycle_on_pagecross; /* boolean. */\n")
	file.WriteString("\tconst unsigned char illegal; /* boolean. */\n")
	file.WriteString("};\n")

	// write out addr str helper
	file.WriteString("\nstatic const char* ADDRESSING_MODE_STR[13] = {\n")
	file.WriteString("\t\"implied\",\n")
	file.WriteString("\t\"accumulator\",\n")
	file.WriteString("\t\"relative\",\n")
	file.WriteString("\t\"immediate\",\n")
	file.WriteString("\t\"absolute\",\n")
	file.WriteString("\t\"absolute,X\",\n")
	file.WriteString("\t\"absolute,Y\",\n")
	file.WriteString("\t\"zeropage\",\n")
	file.WriteString("\t\"zeropage,X\",\n")
	file.WriteString("\t\"zeropage,Y\",\n")
	file.WriteString("\t\"(indirect)\",\n")
	file.WriteString("\t\"(indirect,X)\",\n")
	file.WriteString("\t\"(indirect),Y\"\n")
	file.WriteString("};\n")

	// optable array
	file.WriteString("\nstatic const struct OpcodeInfo OPCODE_INFO_TABLE[0x100] = {\n\t")
	for i, data := range table {
		if (i%0x10) == 0 && i != 0 {
			file.WriteString("\n\t")
		}
		file.WriteString("{\"" + data.Name + "\",\"" + data.Flags.N + data.Flags.Z + data.Flags.C + data.Flags.I + data.Flags.D + data.Flags.V + "\",\"" + data.Summary + "\"," + int2Hex(data.Opcode) + "," + strconv.Itoa(data.AddressingMode) + "," + strconv.Itoa(data.Length) + "," + strconv.Itoa(data.Cycles) + "," + bool2StrInt(data.ExtraCycleOnBranch) + "," + bool2StrInt(data.ExtraCycleOnPageCross) + "," + bool2StrInt(data.Illegal) + "}, ")
	}
	file.WriteString("\n};\n")

	// end include guard
	file.WriteString("\n#endif /* OPCODE_INFO_TABLE_H */\n")
}

func main() {
	tableData, err := ioutil.ReadFile(INFILE)
	if err != nil {
		log.Fatal(err)
	}

	var table []Table
	err = json.Unmarshal(tableData, &table)
	if err != nil {
		log.Fatal(err)
	}

	dump(table)
	dumpDebug(table)

	// ayyyyyy
	println("done!")
}
