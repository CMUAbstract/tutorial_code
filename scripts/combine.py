import argparse
import json
import random
import os

def main(args):
	srcs = args.srcs.split(',')
	data = []
	for src in srcs:
		with open(src, 'r') as f:
			print src
			data += json.load(f)

	random.shuffle(data)
	test_size = int(len(data) * args.test_percent)
	with open(os.path.join(args.dest, 'valid.json'), 'w+') as f:
		json.dump(data[0:test_size], f)
	with open(os.path.join(args.dest, 'test.json'), 'w+') as f:
		json.dump(data[test_size:test_size * 2], f)
	with open(os.path.join(args.dest, 'train.json'), 'w+') as f:
		json.dump(data[test_size * 2:], f)

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument(
		'--srcs',
		type=str,
		help='Source File')
	parser.add_argument(
		'--test_percent',
		type=float,
		default=0.1,
		help='Percentage of data devoted to test & validation')
	parser.add_argument(
		'--dest',
		type=str,
		help='Destination prefix')
	args = parser.parse_args()
	main(args)