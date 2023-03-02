# coding: utf-8
import os  # 导入设置路径的库
import pandas as pd  # 导入数据处理的库
import numpy as np  # 导入数据处理的库

path = "T:/JetBrains/Pycharm/py/MLforAli/ml/"
files = os.listdir(path)  # 读取文件夹下所有文件的路径
for i, name in enumerate(files):  # 逐个遍历
    if name.find("txt") >= 0:  # 判断文件名称中是否包含txt字符
        print(i)  # 输出文件的个数
        os.remove(path + name)  # 删除文件

outfile_id = 0
for k in range(1, 10):
    if k == 1:
        from sklearn import tree

        clf = tree.DecisionTreeRegressor()
        print("决策树回归")
    elif k == 2:
        from sklearn import linear_model

        clf = linear_model.LinearRegression()
        print("线性回归")
    elif k == 3:
        from sklearn.svm import SVC

        clf = SVC(kernel='rbf', probability=True)
        print("SVM回归")
    elif k == 4:
        from sklearn import neighbors

        clf = neighbors.KNeighborsRegressor()
        print("KNN回归")
    elif k == 5:
        from sklearn import ensemble

        clf = ensemble.RandomForestRegressor(n_estimators=30)  # 这里使用20个决策树
        print("随机森林回归")
    elif k == 6:
        from sklearn import ensemble

        clf = ensemble.AdaBoostRegressor(n_estimators=50)  # 这里使用50个决策树
        print("Adaboost回归")
    elif k == 7:
        from sklearn import ensemble

        clf = ensemble.GradientBoostingRegressor(n_estimators=100)  # 这里使用100个决策树
        print("GBRT回归")
    elif k == 8:
        from sklearn.ensemble import BaggingRegressor

        clf = BaggingRegressor(n_estimators=200)
        print("Bagging回归")
    elif k == 9:
        from sklearn.tree import ExtraTreeRegressor

        clf = ExtraTreeRegressor()
        print("ExtraTree极端随机树回归")
    for n in range(1, 11):
        data = pd.read_csv('train.CSV', sep=',', encoding='gb18030')
        if n == 1:
            columns_model = ['card', 'card_rank', 'card_ratio', 'freq', 'freq_rank', 'freq_ratio', 'datamodel_size',
                             'df']
        elif n == 2:
            columns_model = ['card', 'card_rank', 'freq', 'freq_rank', 'df']
        elif n == 3:
            columns_model = ['card', 'card_rank', 'freq', 'freq_rank']
        elif n == 4:
            columns_model = ['card', 'card_ratio', 'freq', 'freq_ratio']
        elif n == 5:
            columns_model = ['card', 'card_ratio', 'freq', 'freq_ratio', 'df']
        elif n == 6:
            columns_model = ['card', 'card_rank', 'card_ratio', 'freq', 'freq_rank', 'freq_ratio']
        elif n == 7:
            columns_model = ['card', 'card_rank', 'card_ratio', 'freq', 'freq_rank', 'freq_ratio', 'df']
        elif n == 8:
            columns_model = ['card', 'card_rank', 'freq', 'freq_rank', 'datamodel_size']
        elif n == 9:
            columns_model = ['card_rank', 'card_ratio', 'freq_rank', 'freq_rank', 'datamodel_size']
        elif n == 10:
            columns_model = ['card_rank', 'card_ratio', 'freq_rank', 'freq_rank']
        print("---------------------------------------------n = ", n)
        X_model = data[columns_model]  # 生成入模自变量
        y = data['y']  # 生成入模因变量
        from sklearn.model_selection import train_test_split  # 导入区分训练集和测试集的模块

        Xtrain, Xtest, Ytrain, Ytest = train_test_split(X_model, y, test_size=0.01)

        # print(len(Xtrain), len(Xtest), len(Ytrain), len(Ytest))

        clf = clf.fit(Xtrain, Ytrain)
        print("------92641--------")
        data = pd.read_csv('test.CSV', sep=',', encoding='gb18030')
        X_model = data[columns_model]  # 生成入模自变量
        y = data['y']  # 生成入模因变量
        # Xtrain0, Xtest0, Ytrain0, Ytest0 = train_test_split(X_model, y, test_size=0.999)
        # print(len(Xtrain0), len(Xtest0), len(Ytrain0), len(Ytest0))
        predictedY0 = clf.predict(X_model)  # 对新数据进行预测
        # print('predictedY:'+str(predictedY0)) # 输出为predictedY:[1]，表示愿意购买，1表示yes
        # print(predictedY0)

        test_1 = 0
        for i in y:
            if i == 1:
                test_1 += 1
        tmp = 0.4
        sum_1 = 0
        diff_10 = 0
        for i in predictedY0:
            if i > tmp:
                sum_1 += 1

        for i, j in zip(y, predictedY0):
            if i == 1 and tmp >= j:
                diff_10 += 1
        # print(len(y),len(predictedY0))
        if diff_10 < 3 and sum_1 < 1500:
            print("test_1:", test_1)
            print("sum_1:", sum_1)
            print("diff_10:", diff_10)
            arr_Y = y.tolist()
            id_arr_hot = [i for i, x in enumerate(arr_Y) if x == 1]
            arr = predictedY0.tolist()
            id_arr = [i for i, x in enumerate(arr) if x > tmp]
            intersection = list(set(id_arr_hot) & set(id_arr))
            print("intersection:", len(intersection))
            # print("id_arr:",id_arr)
            data = pd.read_csv('test.CSV', sep=',', encoding='gb18030')
            columns = ['m', 'k', 'card', 'card_rank', 'card_ratio', 'freq', 'freq_rank', 'freq_ratio', 'datamodel_size',
                       'df', 'y']
            mk = data[columns]  # 生成入模自变量
            m_arr = mk['m'].tolist()
            k_arr = mk['k'].tolist()
            c_arr = mk['card'].tolist()
            ck_arr = mk['card_rank'].tolist()
            cr_arr = mk['card_ratio'].tolist()
            f_arr = mk['freq'].tolist()
            fk_arr = mk['freq_rank'].tolist()
            fr_arr = mk['freq_ratio'].tolist()
            d_arr = mk['datamodel_size'].tolist()
            df_arr = mk['df'].tolist()
            y_arr = mk['y'].tolist()
            filename = str(outfile_id) + "+92641.txt"
            fo = open(filename, "w")
            for id in id_arr:
                # print(m_arr[id],k_arr[id])
                mk_str = m_arr[id] + " " + k_arr[id] + " " + str(c_arr[id]) + " " + str(ck_arr[id]) + " " + str(
                    cr_arr[id]) + " " \
                         + str(f_arr[id]) + " " + str(fk_arr[id]) + " " + str(fr_arr[id]) + " " + str(
                    d_arr[id]) + " " + str(df_arr[id]) + " " + str(y_arr[id]) + "\n"
                fo.write(mk_str)
            # 关闭打开的文件
            fo.close()
            outfile_id += 1

        # print("------92642--------")
        # data = pd.read_csv('92642.csv', sep=',', encoding='gb18030')
        # X_model = data[columns_model]  # 生成入模自变量
        # y = data['y']  # 生成入模因变量
        # predictedY0 = clf.predict(X_model)  # 对新数据进行预测
        # test_1 = 0
        # for i in y:
        #     if i == 1:
        #         test_1 += 1
        # tmp = 0.6
        # sum_1 = 0
        # diff_01 = 0
        # diff_10 = 0
        # for i in predictedY0:
        #     if (i > tmp):
        #         sum_1 += 1
        #
        # idx = 0
        # lost = []
        # for i, j in zip(y, predictedY0):
        #     if (i == 1 and tmp >= j):
        #         diff_10 += 1
        #         lost.insert(0,idx)
        #     idx += 1
        #     # elif(i == 0 and j>0.4):
        #     #     diff_01 += 1
        #     # print(i,j)
        #     # print(Ytest.index(j))
        # if (diff_10 < 1 and sum_1 < 1700):
        #     print("test_1:", test_1)
        #     print("sum_1:", sum_1)
        #     print("diff_10:", diff_10)
        #     arr_Y = y.tolist()
        #     id_arr_hot = [i for i, x in enumerate(arr_Y) if x == 1]
        #     arr = predictedY0.tolist()
        #     id_arr = [i for i, x in enumerate(arr) if x > tmp]
        #     intersection = list(set(id_arr_hot) & set(id_arr))
        #     print("intersection:", len(intersection))
        #     data = pd.read_csv('92642.csv', sep=',', encoding='gb18030')
        #     columns = ['m', 'k','card','card_rank','card_ratio','freq','freq_rank','freq_ratio','datamodel_size','df','y']
        #     mk = data[columns]  # 生成入模自变量
        #     m_arr = mk['m'].tolist()
        #     k_arr = mk['k'].tolist()
        #     c_arr = mk['card'].tolist()
        #     ck_arr = mk['card_rank'].tolist()
        #     cr_arr = mk['card_ratio'].tolist()
        #     f_arr = mk['freq'].tolist()
        #     fk_arr = mk['freq_rank'].tolist()
        #     fr_arr = mk['freq_ratio'].tolist()
        #     d_arr = mk['datamodel_size'].tolist()
        #     df_arr = mk['df'].tolist()
        #     y_arr = mk['y'].tolist()
        #     for id in lost:
        #         print(m_arr[id], k_arr[id])
        #     filename = str(outfile_id)+ "+92642.txt"
        #     fo = open(filename, "w")
        #     for id in id_arr:
        #         mk_str = m_arr[id] + " " + k_arr[id] + " " +str(c_arr[id]) +" " + str(ck_arr[id])+" " + str(cr_arr[id])+" " \
        #                  + str(f_arr[id])+" "+str(fk_arr[id])+" "+str(fr_arr[id])+" "+str(d_arr[id])+" "+str(df_arr[id])+" "+str(y_arr[id])+"\n"
        #         fo.write(mk_str)
        #     # 关闭打开的文件
        #     fo.close()
        #     outfile_id += 1
