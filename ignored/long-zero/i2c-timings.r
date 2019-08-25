
# Delays
plot(hourly$V1, hourly$V2, pch=19, col="saddlebrown", cex=1,
     main="Log of Minutely Max Measure Delay vs Time",
     xlab="Time [min]", ylab="Log minutely max [log ns]")

abline(h=log(10^7 / 2), col=2, lw=8)

legend("topright", c("Delay", "Threshold"), col=c("saddlebrown", 2), lwd=10)
