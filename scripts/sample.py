import os
import json
import argparse
import numpy as np
import pickle
import random

BATCH_SIZE = 32
NUM_SAMPLES = 128
FEATURES = 4
CLASSES = 4

def matify(data):
	labels = []
	examples = []
	for d in data:
		if d['label'] == 0: continue
		labels.append(d['label'] - 1)
		example = []
		idx = len(d['data']) - 1
		while len(example) < NUM_SAMPLES * FEATURES and idx >= 0:
			for e in xrange(len(d['data'][idx]['u'])):
				if len(example) + e > NUM_SAMPLES * FEATURES: break
				example += [d['data'][idx]['u'][e], d['data'][idx]['d'][e], 
					d['data'][idx]['l'][e], d['data'][idx]['r'][e]]

		if len(example) < NUM_SAMPLES * FEATURES: continue
		examples.append(example)

	return examples, labels

def get_batch(data, labels, batch_size=100):
	rand_idx = np.random.choice(data.shape[0], batch_size)
	return data[rand_idx], labels[rand_idx]

def main(args):
	with open(args.test, 'r') as f:
		test_data, test_labels = matify(json.load(f))
	test_data = np.array(test_data, dtype=np.float32)
	test_data /= 256.
	test_labels = np.array(test_labels, dtype=np.float32)
	idx = random.randint(0, test_labels.shape[0])
	print(test_labels[idx]);
	print(idx);        
	bias = np.ones([1, 1])
	weights = test_data[idx].reshape([test_data[idx].shape[0], 1])
	print bias.shape, weights.shape

	if args.param_dir:
		vars_to_save = {'sample.summary': weights, 'ones.summary': bias}
		for var in vars_to_save:
			path = os.path.join(args.param_dir, var)
			with open(path, 'w+') as f:
				pickle.dump(vars_to_save[var], f)

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument(
      '--test',
      type=str,
      help='Test json file')
	parser.add_argument(
      '--param_dir',
      type=str,
      help='Test json file')
	args = parser.parse_args()
	main(args)
