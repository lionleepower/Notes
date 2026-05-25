program hello

   use omp_lib

   print *, "Hello from primary thread."

!$omp parallel

   print *, "Team member ",omp_get_thread_num(), &
  &         "reporting from team of ",omp_get_num_threads(),"."

!$omp end parallel

   print *, "Primary thread finished, goodbye."

end program hello
