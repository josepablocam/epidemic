#a simple analysis of our epidemic simulator#
library(ggplot2)
library(reshape2)
library(plyr)


POP_SIZE <- 100000
SIM_LEN <- 100
CONNECTIVITY <- 10
IMMUNITY <- 0

#low baseline infection rate, medium exposure infection rate, very low mortality, medium length
d1 <- data.frame(base_ir = 1 / 100, exposure_ir = 0.1, mortality = 1 / 1e5, sick_len = 4)
#very low baseline infection rate, high exposure infection rate, high mortality, medium length
d2 <- data.frame(base_ir = 1 / 1000, exposure_ir = 0.2, mortality = 0.4, sick_len = 4)
# low baseline infection rate, low exposure infection rate, medium mortality, long length
d3 <- data.frame(base_ir = 1 / 100, exposure_ir = 0.05, mortality = 0.1, sick_len = 10)

diseases <- list(d1, d2, d3)

simulate <- function(disease){
  command <- sprintf("../simulator -n %i -d %i -c %i -b %f -e %f -m %f -l %i -i %f",
                     POP_SIZE, SIM_LEN, CONNECTIVITY, 
                     disease$base_ir, disease$exposure_ir, disease$mortality, disease$sick_len, IMMUNITY)
   data <- system(command, intern = TRUE)
   read.csv(text = data, header = TRUE)
}
              
results <- lapply(diseases, simulate)
results <- do.call(rbind, results)
results$disease_id <- cumsum(results$day == 1)

results <- melt(results, measure.vars = c("sick", "dead", "healthy"), variable.name = "state")
results <- ddply(results, .(disease_id, day), transform, pct = value / sum(value))

ggplot(results, aes(x = day, y = pct)) +
  geom_line(aes(color = state)) + facet_grid(state ~ disease_id)




