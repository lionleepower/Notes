program main
!   use omp_lib
   integer, parameter   :: N=3400
!   integer, parameter   :: N=6000
!   integer, parameter   :: N=12000
   integer              :: nthreads,mytid,irate,it0,it1,it2
   real*8,dimension(N)  :: x
   real*8,dimension(N,N):: a
   real*8               :: factor,time

   x=0.99
   a=0.95
   factor=0.5

   call system_clock(count_rate=irate)
   call system_clock(it0)
   call system_clock(it1)

!$OMP PARALLEL private(mytid,nthreads)

   nthreads=1   !OMP_GET_NUM_THREADS()
   mytid   =0   !OMP_GET_THREAD_NUM()
   print*,"nthreads,mytid:", nthreads,mytid
   call pwork(nthreads,mytid,factor,N,x,a)
   
!$OMP END PARALLEL

   call system_clock(it2)
   time = dble(it2 - it1*2 + it0)/dble(irate)

   print*," wall-clock time =",time

end program

subroutine pwork(nthreads,mytid,factor,N,x,a)
   integer                :: nthreads,mytid,N
   real*8, dimension(N)   :: x
   real*8, dimension(N,N) :: a
   real*8                 :: factor
   integer                :: ibeg,iend,j,i

   ibeg= 1 + mytid *(N/nthreads)
   iend=(1 + mytid)*(N/nthreads)

   do j = 1,N
      do i = ibeg,iend
         a(i,j) = sqrt(a(i,j)/x(j))*sqrt(x(i)*factor/a(i,j))
      end do
   end do
end subroutine
