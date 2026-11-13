SELECT SYSDATE, NVL(NULL, 'x'), DECODE(1, 1, 'one', 'other') FROM DUAL;
