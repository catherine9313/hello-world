
//#include "stdafx.h"
#include "DynamicConstruct.h"
#include <time.h>
#include <algorithm>
#include "Directories.h"

int median3(int&a, int&b, int&c)
{
	if (a < b) {
		if (b < c) {
			return b;
		}
		else if (a < c) {
			return c;
		}
		else {
			return a;
		}
	}
	else if (a < c) {
		return a;
	}
	else if (b < c) {
		return c;
	}
	else {
		return b;
	}
}

void GridConstructor::Construct(Manage &manage, ConstructCallBack func)
{
	//由于原图太大屏幕不一定显示的下，所以缩放一定比例
	double pK = 0.8;
	const std::string str = "ColorForm.xml";
	ColorMap::ColorModel model = ColorMap::loadColorModel(str);

	manage.GetCurTable()->clear();

	int index = 0;
	MESHINT gridIndexs;
	MESHMAT gridPatches;

	cv::Mat segImg;
	ConstructCallBackParam mCallBackParam;

	int count = 0;
	int state = 0;//0,等待开始；1，等待结束
	clock_t t1;
	clock_t t2;

	while (1)
	{

		char input;
		gridPatches.clear();
		gridIndexs.clear();
		mCallBackParam.mSrc = mCam->getMat();


		/*if (waitKey(1) == 'q')
		break;
		else
		continue;*/
		if (mCallBackParam.mSrc.empty())
			break;
		//t1 = clock();
		if (debug)
		{
			gridPatches = getGridPatchesDebug(mCallBackParam.mSrc, model, mParam, gridIndexs, mCallBackParam.mBlobRes, segImg, mCallBackParam.mStdImg);
			if (mCallBackParam.mBlobRes.empty())
				mCallBackParam.mBlobRes = mCallBackParam.mSrc.clone();
			mCallBackParam.mPatchShow = showGridPatchs(gridPatches, gridIndexs);

			if (mCallBackParam.mPatchShow.empty())
				mCallBackParam.mPatchShow = mCallBackParam.mSrc.clone();

		}
		else
		{
			gridPatches = getGridPatchesRelease(mCallBackParam.mSrc, model, mParam, gridIndexs);

			mCallBackParam.mBlobRes.release();
			mCallBackParam.mPatchShow.release();
			mCallBackParam.mStdImg.release();
		}



		if (func != 0)
			func(mCallBackParam);//调用回调函数

		//if (waitKey(1) == 'q')
		//	break;
		//else
		//	continue;

		set<int> rowIds;
		rowIds.clear();
		int min = 1000;
		int max = -1;
		for (int i = 0; i < gridIndexs.size(); i++)
		{
			int cur = gridIndexs[i][0] / 1000;
			rowIds.insert(gridIndexs[i][0] / 1000);
			if (cur < min)
				min = cur;
			if (cur>max)
				max = cur;
		}

		double avg = (double)(min + max) / 2;
		bool direct = mStartRowID > mEndRowID;

		if (min == -1)
			min = 1000;

		t1 = clock();
		if (state == 0)
		{
			//if (rowIds.find(mStartRowID) != rowIds.end() || rowIds.find(mStartRowID+1) != rowIds.end()|| rowIds.find(mStartRowID-1) != rowIds.end())
			if ((direct&&min != 1000 && min <= mStartRowID) || (!direct && max != -1 && max >= mStartRowID))
				count++;
			if (count >= 1)
			{
				state = 1;
				count = 0;
				std::cout << "begin min=" << min << std::endl;
				if (mAutoRun)
					input = waitKey(1);
				else
					input = waitKey(0);
			}
			else
			{
				if (mAutoRun)
					input = waitKey(1);
				else
					input = waitKey(0);
				//continue;
			}


		}
		else if (state == 1)
		{
			//if (rowIds.find(mEndRowID) != rowIds.end() || rowIds.find(mEndRowID+1) != rowIds.end()|| rowIds.find(mEndRowID-1) != rowIds.end())
			if ((direct&&min != 1000 && min <= mEndRowID) || (!direct && max != -1 && max >= mEndRowID))
				count++;
			if (gridIndexs.size() == 0)
				count++;
			if (count >= 1)
			{
				state = 2;
				count = 0;
				std::cout << "end min=" << min << std::endl;
				if (mAutoRun)
					input = waitKey(1);
				else
					input = waitKey(0);
				break;
			}
			else
			{
				if (mAutoRun)
					input = waitKey(1);
				else
					input = waitKey(0);
				//continue;
			}

		}
		else
			break;
		t2 = clock();
		//cout << "time=" << t2 - t1 << endl;

		if (state == 1)
		{
			//t1 = clock();
			//cout << "update" << endl;
			manage.UpdateCurTable(gridPatches, gridIndexs);
			//cout << "update end" << endl;
			//t2 = clock();
			//cout << "------------------------------update time=" << t2 - t1 << endl;
		}


		if (input == 'q')
			break;
	}
	//cv::Mat show1 = manage.ShowCurTable();
	////调整大小适应显示
	//cv::Mat dis1;
	//cout << "show" << endl;
	//cv::resize(show1, dis1, cv::Size((int)show1.cols*0.6, (int)show1.rows*0.6));

	//manage.ConvertCur2Full();
	//manage.SaveFullTable("full20170609.xml");
	//cv::imshow("show1", show1);//dis1为抽出来的过程中建立的表
	//cv::imwrite("colorNull.jpg", show1);


	//cv::waitKey(0);
}

void GridConstructor::Construct(std::map<int, std::vector<cv::Mat>> &table, ConstructCallBack func)
{
	//由于原图太大屏幕不一定显示的下，所以缩放一定比例
	double pK = 0.8;
	const std::string str = "ColorForm.xml";
	ColorMap::ColorModel model = ColorMap::loadColorModel(str);

	table.clear();

	int index = 0;
	MESHINT gridIndexs;
	MESHMAT gridPatches;

	cv::Mat segImg;
	ConstructCallBackParam mCallBackParam;

	int count = 0;
	int state = 0;//0,等待开始；1，等待结束
	bool checkEmpty=mGetState==0;
	clock_t t1;
	clock_t t2;
	t1 = clock();

	while (1)
	{
		t2 = clock();
		//if (this->isLeft)
		//cout << "time=" << t2 - t1 << endl;
		t1 = t2;
		char input;
		gridPatches.clear();
		gridIndexs.clear();
		mCallBackParam.mSrc = mCam->getMat();
		bool direct = mStartRowID > mEndRowID;
		//cout << "direct:" << direct << endl;
		//cout << "mStartRowID:" << mStartRowID << endl;
		//cout << "mEndRowID:" << mEndRowID << endl;

		/*if (waitKey(1) == 'q')
		break;
		else
		continue;*/
		if (mCallBackParam.mSrc.empty())
		{
			LogOut("empty");
			break;
		}
		//gridIndexs.clear();
		//gridPatches.clear();
		if (debug)
		{
			gridPatches = getGridPatchesDebug(mCallBackParam.mSrc, model, mParam, gridIndexs, mCallBackParam.mBlobRes, segImg, mCallBackParam.mStdImg);
			if (mCallBackParam.mBlobRes.empty())
				mCallBackParam.mBlobRes = mCallBackParam.mSrc.clone();
			mCallBackParam.mPatchShow = showGridPatchs(gridPatches, gridIndexs);

			if (mCallBackParam.mPatchShow.empty())
				mCallBackParam.mPatchShow = mCallBackParam.mSrc.clone();

		}
		else
		{
			gridPatches = getGridPatchesRelease(mCallBackParam.mSrc, model, mParam, gridIndexs);
			mCallBackParam.mPatchShow = showGridPatchs(gridPatches, gridIndexs);

			mCallBackParam.mBlobRes.release();
			mCallBackParam.mPatchShow.release();
			mCallBackParam.mStdImg.release();
		}

		mCallBackParam.isLeft = this->isLeft;

		if (func != 0)
			func(mCallBackParam);//调用回调函数


		//if (waitKey(1) == 'q')
		//	break;
		//else
		//	continue;

		vector<int> rowIds;
		rowIds.clear();
		int min = 1000;
		int max = -1;
		int bound = -1;
		string outCur="rows=";
		for (int i = 0; i < gridIndexs.size(); i++)
		{
			int cur = gridIndexs[i][0] / 1000;
			//cout << "cur:" << cur << endl;
			outCur+= to_string( static_cast<long long>(cur));
			outCur+=" ";
			rowIds.push_back(gridIndexs[i][0] / 1000);

			//rowIds.insert(gridIndexs[i][0] / 1000);
			//if (cur < min)
			//	min = cur;
			//if (cur>max)
			//	max = cur;
			////if (cur == (mStartRowID > mEndRowID ? mStartRowID : mEndRowID))
			bound = gridIndexs[i][gridIndexs[i].size() - 1];

		}
		LogOut(outCur);
		//cout << "bound=" << bound << endl;
		//double avg = (double)(min + max) / 2;

		//20170925
		//加入中值滤波
		vector<int>rightRowIds;
		rightRowIds = rowIds;
		//cout << "rightRowIds:" << endl;

		if (rowIds.size() > 2)
		{
			sort(rowIds.begin(), rowIds.end());
			int median = rowIds[rowIds.size() / 2];
			rowIds = rightRowIds;//还原
			for (int i = 0; i < rowIds.size(); i++)
			{
				if (abs(rowIds[i] - median) < 5)
				{
					rightRowIds[i] = rowIds[i];
				}
				else
				{
					rightRowIds[i] = -100;
				}
			}

			for (int i = 0; i < rightRowIds.size(); i++)
			{
				if (rightRowIds[i] == -100 && i < rightRowIds.size() - 1 && rightRowIds[i + 1] != -100)//是错的，不是最后一位，最后一位不是错的
				{
					rightRowIds[i] = rightRowIds[i + 1] - 1;
				}
				if (rightRowIds[i] == -100 && i == rightRowIds.size() - 1 && rightRowIds[i - 1] != -100)//是错的，是最后一位，倒数第二位不是错的
				{
					rightRowIds[i] = rightRowIds[i - 1] + 1;
				}
				if (rightRowIds[i] < 0)//依然没有改正回来
				{
					rightRowIds[i] = median;
				}

			}

		}
		for (int i = 0; i < rightRowIds.size(); i++)
		{
			if (rightRowIds[i] < min)
				min = rightRowIds[i];
			if (rightRowIds[i] > max)
				max = rightRowIds[i];
			//cout << "rightRowIds" << rightRowIds[i] << endl;

		}


		//int startMinRow = (mStartRowID > mEndRowID) ? mEndRowID : mStartRowID;
		//int startMinRow = -1;

		//direct=0，即往里推时，判断是否有反方向拉出。
		bool stitchFlag = true;//true:检测错误,正常保留行号，拼接 false:确实回退，不保留行号，不拼接
		if (!oldRowIds.empty() && (!direct) && oldStartFlag && !rightRowIds.empty())
		{

			//id decrease
			//if (rightRowIds[0] < oldRowIds[0] && oldRowIds[0] > startMinRow && mStitcher != 0 && !oldSrc.empty())
			if (rightRowIds[0] < oldRowIds[0] && mStitcher != 0 && !oldSrc.empty())
			{
				//stitchFlag = mStitcher->CheckBack(oldSrc, mCallBackParam.mSrc);
				stitchFlag = mStitcher->CheckBack(oldRowIds[0], oldSrc, mCallBackParam.mSrc);
			}
			//id unchange
			//if (rightRowIds[0] == oldRowIds[0] && oldRowIds[0] > startMinRow&& mStitcher != 0 && !oldSrc.empty())
			if (rightRowIds[0] == oldRowIds[0] && mStitcher != 0 && !oldSrc.empty())
				stitchFlag = false;
		}

		bool startFlag = false;
		if (oldRowIds.empty())
		{
			oldRowIds = rightRowIds;
			oldSrc = mCallBackParam.mSrc;
			startFlag = false;
			oldStartFlag = false;
		}
		else if(rightRowIds.size())
		{
			
			if (!direct && oldStartFlag)
			{
				startFlag = true;
			}
			//if (!direct && rightRowIds[0] > oldRowIds[0] && oldRowIds[0] > startMinRow)
			if (!direct && rightRowIds[0] > oldRowIds[0])
			{
				startFlag = true;
				oldStartFlag = true;
			}
			if (stitchFlag)
			{
				oldRowIds = rightRowIds;
				oldSrc = mCallBackParam.mSrc;
			}

		}
		LogOut("end filter");
		//cout << "direct:" << direct << " startflag:" << startFlag << " oldStartFlag:" << oldStartFlag << endl;

		//20170925
		if (min == -1)
			min = 1000;

		t1 = clock();
		if(mGetState!=0)
		{
			if(mGetState(mFloorID)==0)//如果电子锁关上
			{
				LogOut("lock close");
				mCam->closeCamera();
				mGetState=0;
			}
		}
		if (state == 0)
		{
			//if (rowIds.find(mStartRowID) != rowIds.end() || rowIds.find(mStartRowID+1) != rowIds.end()|| rowIds.find(mStartRowID-1) != rowIds.end())
			if ((direct&&min != 1000 && min <= mStartRowID) || (!direct && max != -1 && max >= mStartRowID))
				count+=1;
			else
				count = 0;
			if (count >= 1)
			{
				state = 1;
				count = 0;
				std::cout << "begin min=" << min << " max=" << max << std::endl;
				if (mAutoRun)
					input = waitKey(1);
				else
					input = waitKey(0);
			}
			else
			{
				if (mAutoRun)
					input = waitKey(1);
				else
					input = waitKey(0);
				//continue;
			}


		}
		else if (state == 1)
		{
			//if (rowIds.find(mEndRowID) != rowIds.end() || rowIds.find(mEndRowID+1) != rowIds.end()|| rowIds.find(mEndRowID-1) != rowIds.end())
			if ((direct&&min != 1000 && min <= mEndRowID) || (!direct && max != -1 && max >= mEndRowID))
				count += mMaxMissCount;
			
			if (checkEmpty && gridIndexs.size() == 0)//如果没有注册电子锁，则检测不到的时候，需要那啥
			{
				count++;
				cout << "can not detect rows!" << endl;
			}

			if (count >= mMaxMissCount)
			{
				state = 2;
				count = 0;
				std::cout << "end min=" << min << " max=" << max << std::endl;
				LogOut( "end min=" + to_string((long long)min)+" max="+to_string((long long)max));
				if (mAutoRun)
					input = waitKey(1);
				else
					input = waitKey(0);
				break;
			}
			else
			{
				if (mAutoRun)
					input = waitKey(1);
				else
					input = waitKey(0);
				//continue;
			}

		}
		else
			break;
		t2 = clock();
		//cout << "time=" << t2 - t1 << endl;

		if (state == 1)
		{
			//t1 = clock();
			//cout << "update" << endl;
			UpdateTable(table, gridPatches, gridIndexs, mMinCol, mMaxCol);

			int maxRow = mStartRowID > mEndRowID ? mStartRowID : mEndRowID;
			int minRow = mStartRowID < mEndRowID ? mStartRowID : mEndRowID;
			//if (min != -1 && max != -1 && min >= minRow && max <= maxRow - 4)
			//{
			//cout << "begin stitch" << endl;
			if (mStitcher != 0 && startFlag && stitchFlag)
				mStitcher->OneSideStitch(mCallBackParam.mSrc, direct, bound);
			//mStitcher->OneSideStitch(mCallBackParam.mSrc, 0, bound);

			//cout << "end stitch" << endl;
			//}





			//cout << "update end" << endl;
			//t2 = clock();
			//cout << "------------------------------update time=" << t2 - t1 << endl;
		}


		if (input == 'q')
			break;
	}
	//cv::Mat show1 = manage.ShowCurTable();
	////调整大小适应显示
	//cv::Mat dis1;
	//cout << "show" << endl;
	//cv::resize(show1, dis1, cv::Size((int)show1.cols*0.6, (int)show1.rows*0.6));

	//manage.ConvertCur2Full();
	//manage.SaveFullTable("full20170609.xml");
	//cv::imshow("show1", show1);//dis1为抽出来的过程中建立的表
	//cv::imwrite("colorNull.jpg", show1);


	//cv::waitKey(0);
}