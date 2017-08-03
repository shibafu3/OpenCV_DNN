// opencv_dnn_test.cpp
#include <iostream>
#include <fstream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;

int main(int argc, char** argv) {
	// ImageNet Caffe���t�@�����X���f��
	string protoFile = "bvlc_reference_caffenet/deploy.prototxt";
	string modelFile = "bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel";
	// �摜�t�@�C��
	string imageFile = (argc > 1) ? argv[1] : "images/cat.jpg";
	// Caffe���f���̓ǂݍ���
	cv::Ptr<cv::dnn::Importer> importer;
	try {
		importer = cv::dnn::createCaffeImporter(protoFile, modelFile);
	}
	catch (const cv::Exception& e) {
		cerr << e.msg << endl;
		exit(-1);
	}
	cv::dnn::Net net;
	importer->populateNet(net);
	importer.release();
	// �e�X�g�p�̓��͉摜�t�@�C���̓ǂݍ���
	cv::Mat img = cv::imread(imageFile);
	if (img.empty()) {
		cerr << "can't read image: " << imageFile << endl;
		exit(-1);
	}
	try {
		// ���͉摜�����T�C�Y
		int cropSize = 224;
		cv::resize(img, img, cv::Size(cropSize, cropSize));
		// Caffe�ň���Blob�`���ɕϊ� (���̂�cv::Mat�̃��b�p�[�N���X)
		const cv::dnn::Blob inputBlob = cv::dnn::Blob(img);
		// ���͑w�ɉ摜�����
		net.setBlob(".data", inputBlob);
		// �t�H���[�h�p�X(���`�d)�̌v�Z
		net.forward();
		// �o�͑w(Softmax)�̏o�͂��擾, �����ɗ\�����ʂ��i�[����Ă���
		const cv::dnn::Blob prob = net.getBlob("prob");
		// Blob�I�u�W�F�N�g������Mat�I�u�W�F�N�g�ւ̎Q�Ƃ��擾
		// ImageNet 1000�N���X���̊m��(32bits���������_�l)���i�[���ꂽ1x1000�̍s��(�x�N�g��)
		const cv::Mat probMat = prob.matRefConst();
		// �m��(�M���x)�̍������Ƀ\�[�g���āA���5�̃C���f�b�N�X���擾
		cv::Mat sorted(probMat.rows, probMat.cols, CV_32F);
		cv::sortIdx(probMat, sorted, CV_SORT_EVERY_ROW | CV_SORT_DESCENDING);
		cv::Mat topk = sorted(cv::Rect(0, 0, 5, 1));
		// �J�e�S�����̃��X�g�t�@�C��(synset_words.txt)��ǂݍ���
		// �f�[�^��: categoryList[951] = "lemon";
		vector<string> categoryList;
		string category;
		ifstream fs("synset_words.txt");
		if (!fs.is_open()) {
			cerr << "can't read file" << endl;
			exit(-1);
		}
		while (getline(fs, category)) {
			if (category.length()) {
				categoryList.push_back(category.substr(category.find(' ') + 1));
			}
		}
		fs.close();
		// �\�������J�e�S���Ɗm��(�M���x)���o��
		cv::Mat_<int>::const_iterator it = topk.begin<int>();
		while (it != topk.end<int>()) {
			cout << categoryList[*it] << " : " << probMat.at<float>(*it) * 100 << " %" << endl;
			++it;
		}
	}
	catch (const cv::Exception& e) {
		cerr << e.msg << endl;
	}
	return 0;
}