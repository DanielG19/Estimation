# Estimation
1.- Determine standar deviation for GPS X and Accelerometer X
Watching how number where changing and getting the first 100 numbers and appliying the formula ((measure - mean)*2/100)*(1/2),
Then giving a little more of space for the result, resulting in the next numbers:

![image](https://user-images.githubusercontent.com/29236973/141076284-5263583e-787d-4c23-a40f-8048af0bc22e.png)

2.- Implement a rate gyro attitude integration shceme in UpdateFromIMU().
Dead Reckoning is suceptible to drift, therefore a quaterninan approach is done by using the "IntegratedBodyRate" to get the estimation from time(i) to time (j)

![image](https://user-images.githubusercontent.com/29236973/141078023-4e70dcab-1ec0-446a-ad14-1e2ebabd82a2.png)

3.- Implement all of the elements of the prediction step for the estimator.
-PredictState(): the idea is to multiply the rates for the time diference, but before doing that, it is needed to add -9.81 to z direction, because accelerometers mesure free falling as 0, and hover as 9.81.

![image](https://user-images.githubusercontent.com/29236973/141083949-470448bc-211d-427a-a00e-46af63fcdd45.png)

-GetRbgPrime(): this part is done by using the formula on "Estimation for Quadrators" document.

![image](https://user-images.githubusercontent.com/29236973/141085098-253cc945-5a4d-408a-9c6f-400b950e450e.png)


