FUNC LABEL 1
    MOV STK A VAL 7
    REF STK B STK A
    REF STK C STK A
    MOV REG 0 STK B
    MOV REG 1 STK C
    CAL VAL 7
    RET
FUNC LABEL 2
    MOV STK A VAL 5
    REF STK B STK A
    REF STK C PTR B
    MOV REG 0 STK C
    MOV REG 1 STK B
    CAL VAL 7
    RET
FUNC LABEL 3
    MOV STK A VAL 19
    REF STK B STK A
    REF PTR B STK B
    REF STK B STK B
    MOV REG 0 STK A
    MOV REG 1 STK B
    CAL VAL 7
    RET
FUNC LABEL 4
    MOV STK A VAL 42
    REF STK B STK A
    REF PTR B PTR B
    REF STK C STK B
    MOV REG 0 STK A
    MOV REG 1 STK C
    CAL VAL 7
    RET
FUNC LABEL 7
    MOV REG 2 VAL 1
    NOT REG 1
    ADD REG 1 REG 2
    ADD REG 0 REG 1
    EQU REG 0
    PRINT REG 0
    RET
FUNC LABEL 0
    CAL VAL 1
    CAL VAL 2
    CAL VAL 3
    CAL VAL 4
    RET
