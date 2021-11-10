# Estimation
1.- Determine standar deviation for GPS X and Accelerometer X.

Watching how number where changing and getting the first 100 numbers and appliying the formula ((measure - mean)*2/100)*(1/2),
Then giving a little more of space for the result, resulting in the next numbers:

![image](https://user-images.githubusercontent.com/29236973/141076284-5263583e-787d-4c23-a40f-8048af0bc22e.png)

2.- Implement a rate gyro attitude integration shceme in UpdateFromIMU().

Dead Reckoning is suceptible to drift, therefore a quaterninon approach is done by using the "IntegratedBodyRate" to get the estimation from time(i) to time (j)

![image](https://user-images.githubusercontent.com/29236973/141078023-4e70dcab-1ec0-446a-ad14-1e2ebabd82a2.png)

3.- Implement all of the elements of the prediction step for the estimator.

-PredictState(): the idea is to multiply the rates for the time diference, but before doing that, it is needed to add -9.81 to z direction, because accelerometers mesure free falling as 0, and hover as 9.81.

![image](https://user-images.githubusercontent.com/29236973/141083949-470448bc-211d-427a-a00e-46af63fcdd45.png)

-GetRbgPrime(): this part is done by using the formula on "Estimation for Quadrators" document.

![image](https://user-images.githubusercontent.com/29236973/141085915-326fbab4-9cdb-4f96-9235-7f192e2b7b5e.png)

-Predict(): Here we already have the mean. thus, we only need the covariance, and following the same formulas from de document, we can get the jacobian. also we alredy have the covariance in time(i-1). thus:

![image](https://user-images.githubusercontent.com/29236973/141088716-88c72f94-9d58-4f74-adca-c2091a8af8fd.png)

4.- Implement the Magnetometer Update.

-UpdateFromMag(): get measurement and current estimated, in order to get the difference, and also we normalize from -180 to 180.

![image](https://user-images.githubusercontent.com/29236973/141097414-c6bcb802-5fd3-46c2-9579-7f7dde4bf55d.png)

5.- Implement GPS update:

-UpdateFromGPS(): 



