
adxl$ID <- seq.int(nrow(adxl))
adxl$time <- adxl$ID / 6000


# Z axis
plot(adxl$time, adxl$zAccel, ylim=range(-1.5, -0.5), pch=19, col=4, cex=.1,
     main="Washing Machine Test",
     xlab="Time [min]", ylab="Acceleration [g]")

legend("topleft", c("Z axis"), col=c(4), lwd=10)

# X and Y axes
plot(adxl$time, adxl$xAccel, pch=19, col=3, cex=.1,
     xlab="Time [min]", ylab="Acceleration [g]",
     main="Washing Machine Test (XY axes)")

points(adxl$yAccel ~ adxl$time, pch=19, col=2, cex=.1)

legend("topleft", c("X axis", "Y axis"), col=c(3, 2), lwd=10)



# Zoom in on final spin Z
plot(adxl$time, adxl$zAccel, pch=19, col=4, cex=.1,
     xlim=range(47.0, 50.5),
     ylim=range(-1.5, -0.5),
     xlab="Time [min]", ylab="Acceleration [g]",
     main="Final Spinning Period")

legend("topleft", c("Z axis"), col=c(4), lwd=10)



# Zoom in on final spin XY
plot(adxl$time, adxl$xAccel, pch=19, col=3, cex=.1,
     xlim=range(47.0, 50.5),
     xlab="Time [min]", ylab="Acceleration [g]",
     main="Final Spinning Period")

points(adxl$yAccel ~ adxl$time, pch=19, col=2, cex=.1,
       xlim=range(47.0, 50.5))

legend("topleft", c("X axis", "Y axis"), col=c(3, 2), lwd=10)


# Zoom in on final Y
plot(adxl$time, adxl$yAccel, pch=19, col=2, cex=.1,
     xlim=range(47.0, 50.5),
     xlab="Time [min]", ylab="Acceleration [g]",
     main="Final Spinning Period")

legend("topleft", c("Y axis"), col=c(2), lwd=10)



i2c$ID <- seq.int(nrow(i2c))
i2c$time <- i2c$ID / 6000
i2c$wait <- as.numeric(i2c$V1) / 1000000.0

plot(i2c$time, i2c$wait, pch=19, col="purple", cex=.1,
     xlab="Time [min]", ylab="read time [ms]",
     main="I2C communications interval")




