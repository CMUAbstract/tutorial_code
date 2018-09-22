import os
import json
import argparse
import numpy as np
from tqdm import tqdm
import pickle
from sklearn.preprocessing import normalize
from sklearn.linear_model import SGDClassifier

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
	with open(args.train, 'r') as f:
		train_data, train_labels = matify(json.load(f))
	with open(args.test, 'r') as f:
		test_data, test_labels = matify(json.load(f))
	train_data = np.array(train_data, dtype=np.float32)
	train_data /= 256.
	test_data = np.array(test_data, dtype=np.float32)
	test_data /= 256.
	train_labels = np.array(train_labels, dtype=np.float32)
	test_labels = np.array(test_labels, dtype=np.float32)

	clf = SGDClassifier(loss='hinge', penalty='l2')
	for i in tqdm(xrange(args.training_steps)):
		data, labels = get_batch(train_data, train_labels, BATCH_SIZE)
		clf.partial_fit(data, labels, classes=[c for c in xrange(CLASSES)])
		if((i + 1) % 200 == 0):
			tqdm.write('step %d, training accuracy %g' % (i + 1, clf.score(data, labels)))

	print('Validating...')
	data, labels = get_batch(test_data, test_labels, BATCH_SIZE)
	print('VAL ACCURACY: %f' % clf.score(data, labels))

	if args.param_dir:
		vars_to_save = {'fc1_w.summary': clf.coef_, 'fc1_b.summary': clf.intercept_,}
		for var in vars_to_save:
			path = os.path.join(args.param_dir, var)
			with open(path, 'w+') as f:
				pickle.dump(vars_to_save[var], f)

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument(
      '--train',
      type=str,
      help='Train json file')
	parser.add_argument(
      '--test',
      type=str,
      help='Test json file')
	parser.add_argument(
      '--training_steps',
      type=int,
      default=2000,
      help='Test json file')
	parser.add_argument(
      '--param_dir',
      type=str,
      help='Test json file')
	args = parser.parse_args()
	main(args)
