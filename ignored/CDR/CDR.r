
library(ggplot2)

Quanta <- read.delim(
  "~/Documents/FEMTA/ignored/CDR/real-time-bounds-1.txt", 
  header=FALSE, 
  stringsAsFactors=FALSE)


Quanta$request <- as.factor(Quanta$V3)
Quanta$error <- Quanta$V2 / 1E3

Meta <- data.frame()

Meta.quanta   <- unique(Quanta$V3 / 1E6)
Meta.averages <- with(Quanta, tapply(error, request, mean))
Meta.stddevs  <- with(Quanta, tapply(error, request, sd))
Meta.min      <- with(Quanta, tapply(error, request, min))
Meta.max      <- with(Quanta, tapply(error, request, max))

Meta.Q3       <- with(Quanta, tapply(error, request, quantile, prob=.95))

Meta.averages

Meta.names <- c(".001", ".01", "0.1")

plot(Meta.averages)
plot(Meta.stddevs)
plot(Meta.min)
plot(Meta.max)

stroke <- 0.5

ggplot(Meta, aes(x=Meta.quanta, y=Meta.averages)) +
    geom_ribbon(aes(ymin=Meta.min, ymax=with(Quanta, Meta.max)), 
              alpha=0.0,       #transparency
              #linetype=1,      #solid, dashed or other line types
              #colour="grey70", #border line color
              size=1,          #border line size
              show.legend = TRUE
              ) +
  
  geom_ribbon(aes(ymax=Meta.max,
                  ymin=with(Quanta, tapply(error, request, quantile, prob=.999))), 
              alpha=stroke,       #transparency
              linetype=1,      #solid, dashed or other line types
              size=1,          #border line size
              fill="red") +

  geom_ribbon(aes(ymax=with(Quanta, tapply(error, request, quantile, prob=.999)), 
                  ymin=with(Quanta, tapply(error, request, quantile, prob=.99))), 
            alpha=stroke,       #transparency
            linetype=1,      #solid, dashed or other line types
            size=1,          #border line size
            fill="orange") +

  geom_ribbon(aes(ymax=with(Quanta, tapply(error, request, quantile, prob=.99)),
                  ymin=with(Quanta, tapply(error, request, quantile, prob=.9))), 
            alpha=stroke,       #transparency
            linetype=1,      #solid, dashed or other line types
            size=1,          #border line size
            fill="yellow") +
  
  geom_ribbon(aes(ymax=with(Quanta, tapply(error, request, quantile, prob=.9)),
                  ymin=Meta.min), 
              alpha=stroke,       #transparency
              linetype=1,      #solid, dashed or other line types
              size=1,          #border line size
              fill="green") +
  geom_point(size=2.5, col="black") +
  xlab("Request ms") +
  ylab("Error μs") +
  ggtitle("Request Fidelity", subtitle = waiver()) +
  scale_fill_manual("",values="grey12") +
  
  theme(axis.text=element_text(size=16, face="bold"),
        axis.title=element_text(size=16, face="bold"))
  
  
  
g + theme(legend.title = element_text(colour ="black", size=16, face="bold"))


plot(x=(seq(100, 1, -1) * seq(1, 100, 1)))

legend(x="center",
       title="Probability",
       legend=c(".1%", " 1%", "10%"), 
       col=c("red", "yellow", "green"),
       pch=c(15,15, 15),
       pt.cex=2,
       bg="grey")

