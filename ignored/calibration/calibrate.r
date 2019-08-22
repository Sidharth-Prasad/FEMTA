
library(readr)

GND <- read_csv("pro-GND.txt")
VDD <- read_csv("pro-VDD.txt")
SDA <- read_csv("pro-SDA.txt")
SCL <- read_csv("pro-SCL.txt")

plot(GND$Time, GND$Measure, cex=.1, col=2)
plot(GND$Time, GND$Volts, cex=.1, col=3)

plot(VDD$Time, VDD$Measure, cex=.1, col=2)
plot(VDD$Time, VDD$Volts, cex=.1, col=3)

plot(SDA$Time, SDA$Measure, cex=.1, col=2)
plot(SDA$Time, SDA$Volts, cex=.1, col=3)

plot(SCL$Time, SCL$Measure, cex=.1, col=2)
plot(SCL$Time, SCL$Volts, cex=.1, col=3)

#GND_model <- lm(Volts ~ poly(Measure, 3, raw=TRUE), data=GND)
GND_model <- lm(Volts ~ Measure, data=GND)
VDD_model <- lm(Volts ~ Measure, data=VDD)
SDA_model <- lm(Volts ~ Measure, data=SDA)
SCL_model <- lm(Volts ~ Measure, data=SCL)


GND$Fit   <- -0.0009114640 + 0.0001874118 * GND$Measure
VDD$Fit   <- -0.0009012627 + 0.0001874287 * VDD$Measure
SDA$Fit   <- -0.0005211696 + 0.0001873591 * SDA$Measure
SCL$Fit   <- -0.0009628627 + 0.0001873755 * SCL$Measure

GND$Error <- GND$Volts - GND$Fit
VDD$Error <- VDD$Volts - VDD$Fit
SDA$Error <- SDA$Volts - SDA$Fit
SCL$Error <- SCL$Volts - SCL$Fit

mean(GND$Error)
mean(VDD$Error)
mean(SDA$Error)
mean(SCL$Error)

quantile(GND$Error, .025)
quantile(VDD$Error, .025)
quantile(SDA$Error, .025)
quantile(SCL$Error, .025)

quantile(GND$Error, .975)
quantile(VDD$Error, .975)
quantile(SDA$Error, .975)
quantile(SCL$Error, .975)



plot(GND$Measure ~ GND$Volts, col=3)
plot(GND$Measure ~ GND$Volts, col=3)
summary(GND_model)

sd(resid(GND_model))
mean(resid(GND_model))



plot(GND$Time, GND$Fit, cex=.1, col=4)
plot(GND$Time, GND$Error, cex=.1, col=2)
abline(a=0, b=0, lw=4, col=1)

mean(GND$Error)
sd(GND$Error)
quantile(GND$Error, .025)
quantile(GND$Error, .975)

quantile(GND$Error, .1)
quantile(GND$Error, .9)

plot(GND$Error ~ GND$Time)

tail(GND$Fit)
