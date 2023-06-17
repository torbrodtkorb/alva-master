import os

from hardware import Hardware
from process  import Process

import matplotlib.pyplot as plot
from openpyxl import Workbook

def write_float_numbers_to_excel(filename, data_lists):
    workbook = Workbook()
    sheet = workbook.active

    # Iterate over the data lists
    for column_index, data_list in enumerate(data_lists):
        # Iterate over the float numbers in the data list
        for row_index, number in enumerate(data_list):
            # Write the float number to the Excel cell, offsetting row index by 32 and column index by 1
            sheet.cell(row=row_index+32, column=column_index+2).value = number

    # Save the workbook to the specified filename
    workbook.save(filename)


process = Process("data/p104_aa'_3d")
process.run()
data = process.file_degrees_relative

print("number of poles: ", len(data[0]))
#write_float_numbers_to_excel('capture.xlsx', data)