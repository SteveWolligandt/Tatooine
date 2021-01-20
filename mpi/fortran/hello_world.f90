program hello
  use iso_c_binding
  use c_interface
  !==============================================================================
  integer(c_int), dimension (:,:), allocatable, target:: A, B    
  integer(c_int), dimension (2, 2, 2, 2), target:: arr
  integer:: c, s1, s2     
  !==============================================================================
  call hello_world
  !------------------------------------------------------------------------------
  c = add(1, 2)
  print *,c
  !------------------------------------------------------------------------------
  s1 = 3
  s2 = 4

  allocate(A(s1, s2))
  allocate(B(s1, s2))
  A(:,:) = 1
  B(:,:) = 2
  A(1, 2) = 5
  call add_matrix(c_loc(A), c_loc(B), s1, s2)
  deallocate (A, B)
  !------------------------------------------------------------------------------
  arr(1, 1, 1, 1) =  1; 
  arr(2, 1, 1, 1) =  2; 
  arr(1, 2, 1, 1) =  3; 
  arr(2, 2, 1, 1) =  4; 
  arr(1, 1, 2, 1) =  5; 
  arr(2, 1, 2, 1) =  6; 
  arr(1, 2, 2, 1) =  7; 
  arr(2, 2, 2, 1) =  8; 
  arr(1, 1, 1, 2) =  9; 
  arr(2, 1, 1, 2) = 10; 
  arr(1, 2, 1, 2) = 11; 
  arr(2, 2, 1, 2) = 12; 
  arr(1, 1, 2, 2) = 13; 
  arr(2, 1, 2, 2) = 14; 
  arr(1, 2, 2, 2) = 15; 
  arr(2, 2, 2, 2) = 16; 
  call multi_arr(c_loc(arr))
end program
