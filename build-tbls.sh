#!/bin/bash -e
LEXEGEN_PATH=${LEXEGEN_PATH:-lexegen}
PARSEGEN_PATH=${PARSEGEN_PATH:-parsegen}
$LEXEGEN_PATH src/passes/daisy_parser_pass/daisy.lex --header-file=src/passes/daisy_parser_pass/lex_defs.h --outfile=src/passes/daisy_parser_pass/lex_analyzer.inl
$PARSEGEN_PATH src/passes/daisy_parser_pass/daisy.gr --header-file=src/passes/daisy_parser_pass/parser_defs.h --outfile=src/passes/daisy_parser_pass/parser_analyzer.inl
