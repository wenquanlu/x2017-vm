FUNC LABEL 0
    MOV STK A VAL 223
    MOV REG 0 VAL 212
    NOT REG 0
    MOV STK B VAL 1
    MOV REG 1 STK B
    ADD REG 0 REG 1
    MOV REG 1 STK A
    ADD REG 0 REG 1
    MOV STK C REG 0
    REF STK D STK C
    MOV STK E STK D
    MOV STK F STK E
    MOV STK G STK F
    MOV REG 0 STK G
    PRINT PTR G
    CAL VAL 6
    PRINT VAL 0
    PRINT STK C
    RET
FUNC LABEL 6
    MOV REG 1 VAL 10
    MOV STK A REG 0
    MOV STK B PTR A
    MOV REG 0 STK B
    ADD REG 0 REG 1
    MOV STK C REG 0
    MOV PTR A STK C
    RET
