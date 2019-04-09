Environment:
HLS tool: CyberWorkBench 6.1
system: CentOS 7
Python 3.6

./testimages/:
	5 face images for testing, along with the corresponding tlv files.

./facedetected_vs_ss_sf/facedetected_vs_alg_param.py:
	Find the [detected face/total face] for given test images through SystemC simulation when shift step of the classifier window and the downscaling factor changes. Please note that in this program false positives are NOT considered (only the total face number is checked not the locations) because there are no false positive for the test images. May not give correct results if different test images are used.

./find_latency/find_latency.py:
	Since the HLS tool cannot calculate a reasonable latency of the face detector, real images (test vectors) are used for cycle-accurate simulation to obtain the real latency. The program also do high-level synthesis.

./clean.sh
	remove the files generated after running the scripts.