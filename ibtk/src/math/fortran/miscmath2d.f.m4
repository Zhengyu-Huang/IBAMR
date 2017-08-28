c
c     Routines to compute misc math operations on patches.
c
c     Created on 05 Jan 2004 by Boyce Griffith
c
c     Copyright (c) 2002-2014, Boyce Griffith
c     All rights reserved.
c
c     Redistribution and use in source and binary forms, with or without
c     modification, are permitted provided that the following conditions
c     are met:
c
c        * Redistributions of source code must retain the above
c          copyright notice, this list of conditions and the following
c          disclaimer.
c
c        * Redistributions in binary form must reproduce the above
c          copyright notice, this list of conditions and the following
c          disclaimer in the documentation and/or other materials
c          provided with the distribution.
c
c        * Neither the name of The University of North Carolina nor the
c          names of its contributors may be used to endorse or promote
c          products derived from this software without specific prior
c          written permission.
c
c     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
c     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
c     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
c     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
c     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
c     BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
c     EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
c     TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
c     DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
c     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
c     TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
c     THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
c     SUCH DAMAGE.
c
define(NDIM,2)dnl
define(REAL,`double precision')dnl
define(INTEGER,`integer')dnl
include(SAMRAI_FORTDIR/pdat_m4arrdim2d.i)dnl
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Computes U = alpha V.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine multiply12d(
     &     U,U_gcw,
     &     alpha,
     &     V,V_gcw,
     &     ilower0,iupper0,
     &     ilower1,iupper1)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw,V_gcw

      REAL alpha

      REAL V(CELL2d(ilower,iupper,V_gcw))
c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
c
c     Local variables.
c
      INTEGER i0,i1
c
c     Compute the linear sum.
c
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            U(i0,i1) = alpha*V(i0,i1)
         enddo
      enddo
c
      return
      end
c
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Computes U = alpha V + beta W.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine multiplyadd12d(
     &     U,U_gcw,
     &     alpha,
     &     V,V_gcw,
     &     beta,
     &     W,W_gcw,
     &     ilower0,iupper0,
     &     ilower1,iupper1)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw,V_gcw,W_gcw

      REAL alpha,beta

      REAL V(CELL2d(ilower,iupper,V_gcw))
      REAL W(CELL2d(ilower,iupper,W_gcw))
c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
c
c     Local variables.
c
      INTEGER i0,i1
c
c     Compute the linear sum.
c
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            U(i0,i1) = alpha*V(i0,i1) + beta*W(i0,i1)
         enddo
      enddo
c
      return
      end
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Computes U = A V.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine multiply22d(
     &     U,U_gcw,
     &     A,A_gcw,
     &     V,V_gcw,
     &     ilower0,iupper0,
     &     ilower1,iupper1)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw,A_gcw,V_gcw

      REAL A(CELL2d(ilower,iupper,A_gcw))
      REAL V(CELL2d(ilower,iupper,V_gcw))
c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
c
c     Local variables.
c
      INTEGER i0,i1
c
c     Compute the linear sum.
c
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            U(i0,i1) = A(i0,i1)*V(i0,i1)
         enddo
      enddo
c
      return
      end
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Computes U = A V + beta W.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine multiplyadd22d(
     &     U,U_gcw,
     &     A,A_gcw,
     &     V,V_gcw,
     &     beta,
     &     W,W_gcw,
     &     ilower0,iupper0,
     &     ilower1,iupper1)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw,A_gcw,V_gcw,W_gcw

      REAL beta

      REAL A(CELL2d(ilower,iupper,A_gcw))
      REAL V(CELL2d(ilower,iupper,V_gcw))
      REAL W(CELL2d(ilower,iupper,W_gcw))
c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
c
c     Local variables.
c
      INTEGER i0,i1
c
c     Compute the linear sum.
c
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            U(i0,i1) = A(i0,i1)*V(i0,i1) + beta*W(i0,i1)
         enddo
      enddo
c
      return
      end
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Computes U = A V + B W.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine multiplyadd32d(
     &     U,U_gcw,
     &     A,A_gcw,
     &     V,V_gcw,
     &     B,B_gcw,
     &     W,W_gcw,
     &     ilower0,iupper0,
     &     ilower1,iupper1)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw,A_gcw,V_gcw,B_gcw,W_gcw

      REAL A(CELL2d(ilower,iupper,A_gcw))
      REAL V(CELL2d(ilower,iupper,V_gcw))
      REAL B(CELL2d(ilower,iupper,B_gcw))
      REAL W(CELL2d(ilower,iupper,W_gcw))
c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
c
c     Local variables.
c
      INTEGER i0,i1
c
c     Compute the linear sum.
c
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            U(i0,i1) = A(i0,i1)*V(i0,i1) + B(i0,i1)*W(i0,i1)
         enddo
      enddo
c
      return
      end
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Computes U = |V|_1.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine pwl1norm2d(
     &     U,U_gcw,
     &     V,V_gcw,V_depth,
     &     ilower0,iupper0,
     &     ilower1,iupper1)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw,V_gcw,V_depth

      REAL V(CELL2d(ilower,iupper,V_gcw),0:V_depth-1)
c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
c
c     Local variables.
c
      REAL n
      INTEGER i0,i1,d
c
c     Compute the pointwise norm.
c
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            n = 0.d0

            do d = 0,V_depth-1
               n = n + dabs(V(i0,i1,d))
            enddo

            U(i0,i1) = n
         enddo
      enddo
c
      return
      end
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Computes U = |V|_2.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine pwl2norm2d(
     &     U,U_gcw,
     &     V,V_gcw,V_depth,
     &     ilower0,iupper0,
     &     ilower1,iupper1)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw,V_gcw,V_depth

      REAL V(CELL2d(ilower,iupper,V_gcw),0:V_depth-1)
c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
c
c     Local variables.
c
      REAL n
      INTEGER i0,i1,d
c
c     Compute the pointwise norm.
c
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            n = 0.d0

            do d = 0,V_depth-1
               n = n + V(i0,i1,d)*V(i0,i1,d)
            enddo

            U(i0,i1) = dsqrt(n)
         enddo
      enddo
c
      return
      end
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Computes U = |V|_oo.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine pwmaxnorm2d(
     &     U,U_gcw,
     &     V,V_gcw,V_depth,
     &     ilower0,iupper0,
     &     ilower1,iupper1)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw,V_gcw,V_depth

      REAL V(CELL2d(ilower,iupper,V_gcw),0:V_depth-1)
c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
c
c     Local variables.
c
      REAL n
      INTEGER i0,i1,d
c
c     Compute the pointwise norm.
c
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            n = 0.d0

            do d = 0,V_depth-1
               n = dmax1(n,dabs(V(i0,i1,d)))
            enddo

            U(i0,i1) = n
         enddo
      enddo
c
      return
      end
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c     Carry out fast sweeping algorithm
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      subroutine fastsweep2d(
     &     U,U_gcw,
     &     ilower0,iupper0,
     &     ilower1,iupper1,
     &     dx)
c
      implicit none
c
c     Input.
c
      INTEGER ilower0,iupper0
      INTEGER ilower1,iupper1
      INTEGER U_gcw

c
c     Input/Output.
c
      REAL U(CELL2d(ilower,iupper,U_gcw))
      REAL dx(0:NDIM-1)
c
c     Local variables.
c
      INTEGER i0,i1
      REAL    a,b
      REAL    hx,hy
      REAL    Q,R,S
      REAL    dbar
c
c     Fast-sweeping algorithm
c

      
c     Do the four sweeping directions.
      do i1 = ilower1,iupper1
         do i0 = ilower0,iupper0
            hx = dx(0)
            hy = dx(1)
            a  = dmin1(U(i0-1,i1),U(i0+1,i1))
            b  = dmin1(U(i0,i1-1),U(i0,i1+1))
            
c           Take care of Dirichlet boundaries
            if (a < 0.d0) then
              a  = 0.d0
              hx = hx/2.d0
            endif
            
            if (b < 0.d0) then
              b  = 0.d0
              hy = hy/2.d0
            endif
            
            if (b-a .gt. hx) then
              dbar = a + hx
            
            elseif (a-b .gt. hy) then
              dbar = b + hy
            
            else
              Q = hx*hx + hy*hy
              R = -2.d0*(hy*hy*a + hx*hx*b)
              S = hy*hy*a*a + hx*hx*b*b - hx*hx*hy*hy
              dbar = (-R + sqrt(R*R-4.d0*Q*S))/(2.d0*Q)
            endif
                        
            U(i0,i1) = dmin1(U(i0,i1),dbar)

            enddo
      enddo
      
      do i1 = ilower1,iupper1
         do i0 = iupper0,ilower0,-1
            hx = dx(0)
            hy = dx(1)
            a  = dmin1(U(i0-1,i1),U(i0+1,i1))
            b  = dmin1(U(i0,i1-1),U(i0,i1+1))
            
            if (a < 0.d0) then
              a  = 0.d0
              hx = hx/2.d0
            endif
            
            if (b < 0.d0) then
              b  = 0.d0
              hy = hy/2.d0
            endif
            
            if (b-a .gt. hx) then
              dbar = a + hx
            
            elseif (a-b .gt. hy) then
              dbar = b + hy
            
            else
              Q = hx*hx + hy*hy
              R = -2.d0*(hy*hy*a + hx*hx*b)
              S = hy*hy*a*a + hx*hx*b*b - hx*hx*hy*hy
              dbar = (-R + sqrt(R*R-4.d0*Q*S))/(2.d0*Q)
            endif
            
            U(i0,i1) = dmin1(U(i0,i1),dbar)
         enddo
      enddo
      
      do i1 = iupper1,ilower1,-1
         do i0 = iupper0,ilower0,-1
            hx = dx(0)
            hy = dx(1)
            a  = dmin1(U(i0-1,i1),U(i0+1,i1))
            b  = dmin1(U(i0,i1-1),U(i0,i1+1))
            
            if (a < 0.d0) then
              a  = 0.d0
              hx = hx/2.d0
            endif
            
            if (b < 0.d0) then
              b  = 0.d0
              hy = hy/2.d0
            endif
            
            if (b-a .gt. hx) then
              dbar = a + hx
            
            elseif (a-b .gt. hy) then
              dbar = b + hy
            
            else
              Q = hx*hx + hy*hy
              R = -2.d0*(hy*hy*a + hx*hx*b)
              S = hy*hy*a*a + hx*hx*b*b - hx*hx*hy*hy
              dbar = (-R + sqrt(R*R-4.d0*Q*S))/(2.d0*Q)
            endif
            
            U(i0,i1) = dmin1(U(i0,i1),dbar)
         enddo
      enddo
      
      do i1 = iupper1,ilower1,-1
         do i0 = ilower0,iupper0
            hx = dx(0)
            hy = dx(1)
            a  = dmin1(U(i0-1,i1),U(i0+1,i1))
            b  = dmin1(U(i0,i1-1),U(i0,i1+1))
            
            if (a < 0.d0) then
              a  = 0.d0
              hx = hx/2.d0
            endif
            
            if (b < 0.d0) then
              b  = 0.d0
              hy = hy/2.d0
            endif
            
            if (b-a .gt. hx) then
              dbar = a + hx
            
            elseif (a-b .gt. hy) then
              dbar = b + hy
            
            else
              Q = hx*hx + hy*hy
              R = -2.d0*(hy*hy*a + hx*hx*b)
              S = hy*hy*a*a + hx*hx*b*b - hx*hx*hy*hy
              dbar = (-R + sqrt(R*R-4.d0*Q*S))/(2.d0*Q)
            endif
            
            U(i0,i1) = dmin1(U(i0,i1),dbar)
         enddo
      enddo

      return
      end
c
c
