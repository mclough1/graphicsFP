CSCI441 - Lab 11
Joseph Kim / Joseph Joestar / josephkim@mymail.mines.edu

Q1. Roll forward without stopping. Rolling in the correct direction though (ie. not rolling backwards, but moving forward)
Q2. Yes: (-1,  0,  0) for x >  GROUND_SIZE
		 ( 1,  0,  0) for x < -GROUND_SIZE
		 ( 0,  0, -1) for z >  GROUND_SIZE
		 ( 0,  0,  1) for z < -GROUND_SIZE
Q3. The two normals can be calculated by taking the distance vectors between the two marble locations.
		L_1 - L_2 = vector from 2 to 1, which would be the normal vector to use to recalculate distance vector of 1 (N_1).
		L_2 - L_1 = vector from 1 to 2, which would be the normal vector to use to recalculate distance vector of 2 (N_2).
		These vectors would would have to be normalized as well.
Q4. Yes, although, if two marbles' initial positions result in them being within each other, they can start off stuck together and static.
Q5. 8
Q6. Write-up was good.
Q7. ~1 hr
Q8. N/A