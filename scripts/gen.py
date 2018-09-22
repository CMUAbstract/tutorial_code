import argparse
import pickle
import os
import numpy as np

def gen_header(data, dir):
	rows, cols = data.shape
	data_str = []
	for row in data.tolist():
		data_str += map(lambda x: 'F_LIT(' + str(x) + ')', row)

	data_str = str(data_str).replace(']', '}').replace('[', '{').replace("'", '')
	with open(os.path.join(dir, 'data.h'), 'w+') as f:
		f.write('#ifndef DATA_H\n#define DATA_H\n')
		f.write('#include <libfixed/fixed.h>\n')
		f.write('#include <libmsp/mem.h>\n')
		f.write('__nv fixed data[' + str(rows) + '][' + str(cols) + '] = ' + data_str + ';\n')
		f.write('#endif\n')

def main(args):
	with open(args.weights, 'r') as f:
		weights = pickle.load(f)
	with open(args.bias, 'r') as f:
		bias = pickle.load(f)

	data = np.column_stack((weights,bias))
	gen_header(data, args.header_dir)

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument(
      '--weights',
      type=str,
      help='Weight file')
	parser.add_argument(
      '--bias',
      type=str,
      help='Bias file')
	parser.add_argument(
      '--header_dir',
      type=str,
      help='Output header directory')
	args = parser.parse_args()
	main(args)
