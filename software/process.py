import os
import numpy
import math
import scipy.signal as signal

class Process:
    file_paths            = []
    file_samples          = []
    file_poles            = []
    file_degrees_relative = []
    file_degrees          = []

    # file_* 
    # file_ett_eller_annet[file_index][sample_index]
    # iterer poles i fil 5
    # for pole in file_poles[5]:

    standard_deviations   = []
    means                 = []
    ranges                = []
    percent_of_pole_pairs = []

    samples_per_revolution = None
    path                   = None
    pole_count             = None

    def __init__ (self, path: str):
        self.path = path
        self.samples_per_revolution = 54857
        self.pole_count = 18

    def run (self):
        self.file_paths.clear()
        self.file_samples.clear()
        self.file_poles.clear()
        self.file_degrees_relative.clear()
        self.file_degrees.clear()
        self.standard_deviations.clear()
        self.means.clear()
        self.ranges.clear()
        self.percent_of_pole_pairs.clear()

        # Get list of files to process
        path = self.path
        if os.path.isfile(path):
            self.file_paths.append(path)
        elif os.path.isdir(path):
            self.file_paths = [os.path.join(path, filename) for filename in os.listdir(path) if os.path.isfile(os.path.join(path, filename))]

        status = True

        # Process the files
        for path in self.file_paths:
            # Load the file
            with open(path, 'r') as file:
                lines = file.readlines()
                samples = numpy.array([float(line.strip()) for line in lines])
            
            # Center the signal
            average = numpy.mean(samples)
            samples -= average

            # Filter using 6th-order Butterworth
            z, p, k = signal.butter(N = 6, Wn = 3, fs = 7500, btype = 'low', output='zpk')
            samples = signal.sosfilt(signal.zpk2sos(z, p, k), samples)
            self.file_samples.append(samples)

            # Find the indices of local maxima (peaks)
            poles = signal.argrelextrema(samples, numpy.greater)[0].tolist()

            # Discard the first pole, and keep N poles for processing
            if not len(poles) >= self.pole_count + 1:
                status = False
                
            
            poles = poles[1 : 1 + self.pole_count]

            # Convert to relative offset from first pole
            poles = [i - poles[0] for i in poles]

            # Convert to useful information
            degrees = [i * 2 * math.pi / self.samples_per_revolution for i in poles]
            differences = [degrees[i] - degrees[i-1] for i in range(1, len(degrees))]

            self.file_poles.append(poles)
            self.file_degrees_relative.append(differences)
            self.file_degrees.append(degrees)
        
        if not status:
            return False
        
        return True

        # Compute statistics per pole
        for index in range(len(self.file_degrees_relative[0])):
            data = [degrees_relative[index] for degrees_relative in self.file_degrees_relative]

            # Compute standard deviation
            self.standard_deviations.append(numpy.std(data))

            # Compute mean value
            self.means.append(numpy.mean(data))

            #                                     +-----+
            # Stator 1:        pol1: 5392   pol2: | 939 |    pol3:-8393093    ...
            # Stator 2:        pol1: 5392   pol2: | 939 |    pol3:-8393093    ...
            # Stator 3:        pol1: 5392   pol2: | 939 |    pol3:-8393093    ...
            # Stator 4:        pol1: 5392   pol2: | 939 |    pol3:-8393093    ...
            # Stator 5:        pol1: 5392   pol2: | 939 |    pol3:-8393093    ...
            # Stator 6:        pol1: 5392   pol2: | 939 |    pol3:-8393093    ...
            # Stator 7:        pol1: 5392   pol2: | 939 |    pol3:-8393093    ...
            #                                     +-----+

            # Compute range
            self.ranges.append(numpy.max(data) - numpy.min(data))
        
        return True
