      SUBROUTINE CALC1
C
C        COMPUTE CAPITAL  U, CAPITAL V, Z AND H
C
      IMPLICIT REAL*8	(A-H, O-Z)
      PARAMETER (N1=1335, N2=1335)

      COMMON  U(N1,N2), V(N1,N2), P(N1,N2),
     *        UNEW(N1,N2), VNEW(N1,N2),
     1        PNEW(N1,N2), UOLD(N1,N2),
     *        VOLD(N1,N2), POLD(N1,N2),
     2        CU(N1,N2), CV(N1,N2),
     *        Z(N1,N2), H(N1,N2), PSI(N1,N2)
C
      COMMON /CONS/ DT,TDT,DX,DY,A,ALPHA,ITMAX,MPRINT,M,N,MP1,
     1              NP1,EL,PI,TPI,DI,DJ,PCF
C
      FSDX = 4.D0/DX + 1
      FSDY = 4.D0/DY + 1

C SPEC removed CCMIC$ DO GLOBAL
      DO 100 J=1,N
      DO 100 I=1,M
      U(I,J) = .5D0*(V(I+1,J)+V(I,J-1))*U(I-1,J)
      V(I,J) = .5D0*(U(I,J-1)+U(I,J))*V(I,J)
  100 CONTINUE

      RETURN
      END
