FUNC LABEL 0
    MOV STK A VAL 5
    REF REG 0 STK A
    CAL VAL 2
    RET
FUNC LABEL 2
    MOV REG 1 VAL 1
    NOT REG 1
    MOV REG 2 VAL 1
    ADD REG 1 REG 2
    MOV STK A REG 0
    PRINT PTR A
    MOV REG 2 PTR A
    ADD REG 2 REG 1
    MOV PTR A REG 2
    EQU REG 2
    ADD REG 7 REG 2
    CAL VAL 2
    RET