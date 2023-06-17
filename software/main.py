import os

from hardware import Hardware
from process  import Process

import matplotlib.pyplot as plot

hardware = Hardware('COM6')
process  = Process('data/tmp')

measurements = []

def delete_tmp_content ():
    try:
        path = 'data/tmp'
        if os.path.exists(path) and os.path.isdir(path):
            filenames = os.listdir(path)
            for filename in filenames:
                file_path = os.path.join(path, filename)
                if os.path.isfile(file_path):
                    os.remove(file_path)
    except:
        pass

def try_create_tmp ():
    if not os.path.exists('data/tmp'):
        os.makedirs('data/tmp')

def main ():
    try_create_tmp()
    delete_tmp_content()

    index = 0
    delete_tmp_when_receiving_data = True

    while True:
        status, data = hardware.get_packet()
        if not status:
            pass # CRC error
    
        if len(data) == 1:
            code = data[0]
            if code == 0:
                # Complete
                process.run()


                # Du har prossesert data
                plot.figure()
                cmap = plot.get_cmap('tab10')

                for i, samples in enumerate(process.file_samples):
                    plot.plot(samples, color = cmap(i), label = 'Index %d' % i)
                
                plot.legend()
                plot.show()

                delete_tmp_when_receiving_data = True
                measurements.clear()
                index = 0
                pass
            elif code == 1:
                # Abort
                delete_tmp_when_receiving_data = True
                measurements.clear()
                index = 0
                pass
        else:
            if delete_tmp_when_receiving_data:
                delete_tmp_when_receiving_data = False
                delete_tmp_content()

            # Data is received
            data = hardware.bytes_to_samples(data)
            measurements.append(data)

            try_create_tmp()
            with open('data/tmp/raw_%d.txt' % index, "w") as file:
                for number in data:
                    file.write(str(number) + "\n")
            
            index += 1

main()