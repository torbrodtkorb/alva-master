from process  import Process
import matplotlib.pyplot as plot

process  = Process('data/tiss_single')

process.run()

# Du har prossesert data
plot.figure()
cmap = plot.get_cmap('tab10')

for i, samples in enumerate(process.file_samples):
    plot.plot(samples, color = cmap(i), label = 'Index %d' % i)

plot.legend()
plot.show()