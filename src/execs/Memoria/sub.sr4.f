      subroutine calc1
C       
C       COMPUTE CAPITAL  U, CAPITAL V, Z AND H
C       
        implicit real*8 (a-h, o-z)
        parameter (n1 = 1335, n2 = 1335)
        
        common u(n1, n2), v(n1, n2), p(n1, n2), unew(n1, n2), vnew(n1, n
     *2), pnew(n1, n2), uold(n1, n2), vold(n1, n2), pold(n1, n2), cu(n1,
     * n2), cv(n1, n2), z(n1, n2), h(n1, n2), psi(n1, n2)
C       
        common /cons/ dt, tdt, dx, dy, a, alpha, itmax, mprint, m, n, mp
     *1, np1, el, pi, tpi, di, dj, pcf
C       
        fsdx = 4.d0 / dx + 1
        fsdy = 4.d0 / dy + 1
        
C       SPEC removed CCMIC$ DO GLOBAL
        do j = 1, n
          do i = 1, m
            cu(i + 1, j) = .5d0 * (p(i + 1, j) + p(i, j)) * u(i + 1, j)
            cv(i, j + 1) = .5d0 * (p(i, j + 1) + p(i, j)) * v(i, j + 1)
            z(i + 1, j + 1) = (fsdx * (v(i + 1, j + 1) - v(i, j + 1)) - 
     *fsdy * (u(i + 1, j + 1) - u(i + 1, j))) / (p(i, j) + p(i + 1, j) +
     * p(i + 1, j + 1) + p(i, j + 1))
            h(i, j) = p(i, j) + .25d0 * (u(i + 1, j) * u(i + 1, j) + u(i
     *, j) * u(i, j) + v(i, j + 1) * v(i, j + 1) + v(i, j) * v(i, j))
          enddo
        enddo
        
        return
      end
