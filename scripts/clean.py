import argparse
import json

def main(args):
	with open(args.src, 'r') as f:
		data = [[{'u':[], 'd':[], 'l':[], 'r':[]}]]
		labels = []
		qd = 0
		for l in f:
			line = l.rstrip()
			if line.startswith('U:'):
				qd += 1
				data[-1][-1]['u'] = map(int, line[2:-1].split(','))
			if line.startswith('D:'):
				qd += 1
				data[-1][-1]['d'] = map(int, line[2:-1].split(','))
			if line.startswith('L:'): 
				qd += 1
				data[-1][-1]['l'] = map(int, line[2:-1].split(','))
			if line.startswith('R:'): 
				qd += 1
				data[-1][-1]['r'] = map(int, line[2:-1].split(','))
			if qd == 4:
				data[-1].append({'u':[], 'd':[], 'l':[], 'r':[]})
				qd = 0
			if line.startswith('*****'): 
				label = int(filter(str.isdigit, line))
				labels.append(label)
				qd = 0
				data[-1] = data[-1][:-1]
				data.append([{'u':[], 'd':[], 'l':[], 'r':[]}])

		data = data[:-1]
		print 'len(data):', len(data)
		print 'len(labels):', len(labels)

		coalesced_data = []
		for d, l in zip(data, labels):
			coalesced_data.append({'label': l, 'data': d})

		data = coalesced_data
		if args.dest:
			with open(args.dest, 'w+') as f:
				json.dump(data, f)


if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument(
		'--src',
		type=str,
		help='Source File')
	parser.add_argument(
		'--dest',
		type=str,
		help='Destination JSON File')
	args = parser.parse_args()
	main(args)