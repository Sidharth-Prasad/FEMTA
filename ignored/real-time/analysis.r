

real$ID <- seq.int(nrow(real))
real$time <- real$ID / 1200

real$Error <- real$V2 / 1000

plot(real$time, real$Error, pch=19, col=4, cex=.1,
     main="Real Time Kernel Performance",
     xlab="Time [min]", ylab="Request Error [μs]")

mean(real$Error)
max(real$Error)
