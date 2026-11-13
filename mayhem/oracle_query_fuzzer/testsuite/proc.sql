CREATE OR REPLACE PROCEDURE raise_sal(p_id IN NUMBER, p_pct IN NUMBER) AS
  v_sal emp.sal%TYPE;
BEGIN
  SELECT sal INTO v_sal FROM emp WHERE empno = p_id;
  UPDATE emp SET sal = v_sal * (1 + p_pct / 100) WHERE empno = p_id;
EXCEPTION
  WHEN NO_DATA_FOUND THEN NULL;
END;
