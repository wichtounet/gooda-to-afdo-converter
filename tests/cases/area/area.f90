MODULE Triangle_Operations
     IMPLICIT NONE
CONTAINS
     FUNCTION Area(x,y,z)
      REAL(8) :: Area            ! function type
      REAL(8), INTENT( IN ) :: x, y, z
      REAL(8) :: theta, height
      theta = ACOS((x**2+y**2-z**2)/(2.0*x*y))
      height = x*SIN(theta); Area = 0.5*y*height
     END FUNCTION Area
END MODULE Triangle_Operations

PROGRAM Triangle
     USE Triangle_Operations
     IMPLICIT NONE
     REAL(8) :: a, b, c, area_sum

     INTEGER :: a_i, b_i, c_i
    
     area_sum = 0.0

     DO a_i = 1, 1000, 1 
        a = a_i * 1.0
        DO b_i = 1, 1000, 1 
            b = b_i * 1.0
            DO c_i = 1, 1000, 1 
                c = c_i * 1.0
                area_sum = area_sum + Area(a, b, c)
            END DO
        END DO
     END DO

     PRINT *, 'Triangle''s area:  ', area_sum 

END PROGRAM Triangle
