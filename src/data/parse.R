df<-read.csv("aepsweep.csv")
df<-df[df$sccess_size == 0 || df$access_size == df$stride_size,]
df<-df[df$rerun==0,]
#df<-df[df$media == 'lpmem']
df<-df[df$avgpower == 15000,]
#df<-df[df$task_type=="RandBW",]
#vars = c("access_size", "media", "operation", "prefetching", "stride_size", "task_type", "threads", "throughput", "time")
#df1<-df[vars]
rpivotTable::rpivotTable(data = df, header=FALSE)

