import os

from hardware import Hardware
from process  import Process

import matplotlib.pyplot as plot

hardware = Hardware('COM6')

measurements = []

def delete_in_folder_content (path):
    try:
        if os.path.exists(path) and os.path.isdir(path):
            filenames = os.listdir(path)
            for filename in filenames:
                file_path = os.path.join(path, filename)
                if os.path.isfile(file_path):
                    os.remove(file_path)
    except:
        pass

def get_stator_details():
    print()
    name = input('Stator name: ')
    mode = input('3D scan (y/N): ')

    is_3d = mode.lower() == 'y'

    if is_3d:
        hardware.send_code(hardware.MODE_3D)
    else:
        hardware.send_code(hardware.MODE_SINGLE)
    
    filename = "tmp" if not name else (name + '_3d') if is_3d else (name + '_single')
    
    directory = "data/" + filename
    os.makedirs(directory, exist_ok=True)
    delete_in_folder_content(directory)
    
    print('waiting for start button to be pressed')
    hardware.send_code(hardware.START)

    return filename

def main ():
    index = 0
    delete_tmp_when_receiving_data = True

    filename = get_stator_details()

    while True:
        status, data = hardware.get_packet()
        if not status:
            pass # CRC error
        
        if len(data) == 1:
            code = data[0]
            print('received a short packet with code %d' % code)
            print('done reading %d measurements' % len(measurements))
            if code == 0:
                # Complete
                process  = Process('data/' + filename)
                process.run()

                # Du har prossesert data
                plot.figure()
                cmap = plot.get_cmap('tab10')

                for i, samples in enumerate(process.file_samples):
                    plot.plot(samples, color = cmap(i), label = 'Index %d' % i)
                
                plot.legend()
                plot.show()
                measurements.clear()
                index = 0
            elif code == 1:
                # Abort
                measurements.clear()
                index = 0

            filename = get_stator_details()
            print()
        else:
            print('received a data packet')

            # Data is received
            data = hardware.bytes_to_samples(data)
            measurements.append(data)

            print('data/%s/%d.txt' % (filename, index))
            print()
            with open('data/%s/%d.txt' % (filename, index), "w") as file:
                for number in data:
                    file.write(str(number) + "\n")
            
            index += 1

main()