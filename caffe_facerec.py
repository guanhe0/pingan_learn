# -*- coding: utf-8 -*-
"""
Created on Tue. Mar 07 16:59:55 2017

@author: wujiyang
@brief：读取lfw数据库6000对人脸数据进行精确度验证
"""
import numpy as np
import matplotlib.pyplot as plt
import os
import sys
import sklearn
import sklearn.metrics.pairwise as pw

caffe_root = '/home/wujiyang/caffe/'  
sys.path.insert(0, caffe_root + 'python')
import caffe

#图片库路径，若label.txt中使用的是绝对路径，则无需修改
imageBasePath = ''

#模型初始化相关操作
def initilize():
    print 'model initilizing...'
    deployPrototxt = "./vgg-face-deploy.prototxt"
    modelFile = "./vgg-face.caffemodel"
    caffe.set_mode_gpu()
    caffe.set_device(0)
    net = caffe.Net(deployPrototxt, modelFile,caffe.TEST)
    return net

#读取图像标签
def readImagelist(labelFile):
    '''
    @brief：从列表文件中，读取图像数据到矩阵文件中
    @param： labelfile 图像列表文件
    @return ：需要对比的图片路径，以及是否为同一个人的标记
    labelfile的格式说明：每一行的数据如下所示，左边分别为要对比的图片，右边为是否是同一个人的标记
    同一个表示为1，不同人表示为0
    img1 img2  label
    '''
    file = open(labelFile)
    lines = file.readlines()
    file.close()
    left = []
    right = []
    labels = []
    for line in lines:
        path = line.strip('\n').split('\t')
        #read left image
        left.append(path[0])
        #read right image
        right.append(path[1])
        #read label
        labels.append(int(path[2]))

    assert(len(left) == len(right))
    assert(len(right) == len(labels))

    return left, right, labels

#特征提取，使用caffe的io接口
def extractFeature(leftImageList, rightImageList, net):
    transformer = caffe.io.Transformer({'data': net.blobs['data'].data.shape}) # create transformer for the input called 'data'

    transformer.set_transpose('data', (2,0,1)) # move image channls to outermost dimension
    #transformer.set_mean('data', np.load(caffe_root + meanFile).mean(1).mean(1)) # mean pixel
    transformer.set_raw_scale('data', 255)  # rescale from [0,1] to [0,255]
    transformer.set_channel_swap('data', (2,1,0)) # swap channels from RGB to BGR
    # set net to batch size of 1 如果图片较多就设置合适的batchsize
    net.blobs['data'].reshape(1, 3,224, 224)  

    leftfeature = []
    rightfeature = []
    for i in range(len(leftImageList)):
        #分别读取左右列图像数据

        #读取左边图像,并提取特征
        imageleft = os.path.join(imageBasePath, leftImageList[i])
        img = caffe.io.load_image(imageleft)
        #print imageleft, img.shape
        img = caffe.io.resize_image(img, (224,224))
        #print imageleft, img.shape
        net.blobs['data'].data[...] = transformer.preprocess('data', img)
        #print net.blobs['data'].data.shape
        out = net.forward()
        #featureleft = np.float64(out['fc7'])
        #print featureleft.shape
        feature = np.float64(net.blobs['fc7'].data)
        #print feature, type(feature)
        leftfeature.append(feature)

        #读取右边图像,并提取特征
        imageright = os.path.join(imageBasePath, rightImageList[i])
        img = caffe.io.load_image(imageright)
        img = caffe.io.resize_image(img, (224,224))
        net.blobs['data'].data[...] = transformer.preprocess('data', img)
        out = net.forward()
        feature = np.float64(net.blobs['fc7'].data)
        rightfeature.append(feature)

    return leftfeature, rightfeature  #list

#计算精确度
def calculate_accuracy(distance, labels, num):    
    '''
    #计算识别率,
    选取阈值，计算识别率
    '''    
    accuracy = {}
    predict = np.empty((num,))
    threshold = 0.1
    while threshold <= 0.9 :
        for i in range(num):
            if distance[i] >= threshold:
                predict[i] = 0
            else:
                predict[i] = 1
        predict_right =0.0
        for i in range(num):
            if predict[i] == labels[i]:
               predict_right += 1.0
        current_accuracy = (predict_right / num)
        accuracy[str(threshold)] = current_accuracy
        threshold = threshold + 0.001
    #将字典按照value排序
    temp = sorted(accuracy.items(), key = lambda d:d[1], reverse = True)
    highestAccuracy = temp[0][1]
    thres = temp[0][0]
    return highestAccuracy, thres

#绘制roc曲线
def draw_roc_curve(fpr,tpr,title='cosine',save_name='roc_lfw'):
    #绘制roc曲线
    plt.figure()
    plt.plot(fpr, tpr)
    plt.plot([0, 1], [0, 1], 'k--')
    plt.xlim([0.0, 1.0])
    plt.ylim([0.0, 1.0])
    plt.xlabel('False Positive Rate')
    plt.ylabel('True Positive Rate')
    plt.title('Receiver operating characteristic using: '+title)
    plt.legend(loc="lower right")
    plt.show()
    #plt.savefig(save_name+'.png')



if __name__ == '__main__':

    #模型初始化
    net = initilize()

    #读取文件列表
    print "reading imagelist..."
    leftImageList, rightImageList, labels = readImagelist('tmp.txt')

    #提取特征
    print "feature extracting..."
    leftfeature_list, rightfeature_list = extractFeature(leftImageList, rightImageList, net)

    #将特征list变为2维ndarray
    leftfeature = np.reshape(leftfeature_list,(len(labels),4096))
    rightfeature = np.reshape(rightfeature_list,(len(labels),4096))

    #print leftfeature.shape, rightfeature.shape
    #print leftfeature, type(leftfeature)

    #进行距离计算  cosine距离
    print "computing distance..."
    dis = pw.pairwise_distances(leftfeature, rightfeature, metric='cosine')
    distance = np.empty((len(labels),))
    for i in range(len(labels)):
        distance[i] = dis[i][i]
    print 'Distance before normalization:\n', distance
    print 'Distance max:', np.max(distance), 'Distance min:', np.min(distance), '\n'

    #距离需要归一化到0-1,与标签0-1匹配
    distance_norm = np.empty((len(labels),))
    for i in range(len(labels)):
        distance_norm[i] = (distance[i]-np.min(distance))/(np.max(distance)-np.min(distance)) #此处计算时会有误差产生，理论上应该有1的，然而却没有！
    print 'Distance after normalization:\n', distance_norm
    print 'Distance_norm max:', np.max(distance_norm), 'Distance_norm min:', np.min(distance_norm), '\n'

    #根据label和distance_norm计算精确度
    highestAccuracy, threshold = calculate_accuracy(distance_norm,labels,len(labels))
    print ("The highest accuracy is : %.4f, and the corresponding threshold is %s \n"%(highestAccuracy, threshold))

    #绘制roc曲线
    fpr, tpr, thresholds = sklearn.metrics.roc_curve(labels, distance_norm)
    draw_roc_curve(fpr, tpr, title='cosine',save_name='lfw_evaluate')