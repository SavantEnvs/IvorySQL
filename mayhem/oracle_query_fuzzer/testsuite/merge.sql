MERGE INTO emp e USING dept d ON (e.deptno = d.deptno) WHEN MATCHED THEN UPDATE SET e.sal = e.sal + 1;
