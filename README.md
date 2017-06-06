# 2-center

An implementation of [A Near-Linear Algorithm for the Planar 2-Center Problem](http://www.math.tau.ac.il/~michas/center2new.pdf)


## Compilation

This project should be built by `Qt >=5.7.0`, other versions are not tested.

Please switch to `Release` configuration for higher performance.


## Manual

1. 操作
	1. 新增点

		鼠标左键单击

	2. 删除点

		鼠标右键单击

	3. 缩放

		鼠标滚轮

	4. 移动视角

		鼠标左键拖动

2. 动画
	1. 播放调节
		1. 开始、暂停、停止

			如按钮文字所示

		2. 播放速度

			拖动 Speed 滑块

		3. 修改播放进度

			拖动进度条滑块

		4. 其它说明

			在播放过程中不能修改点，需要先停止

	2. 1-center problem

		演示随机增量算法，标示的圆是当前覆盖条件的最小圆，空心点是已检验的点和正在检验的点

	3. 2-center problem

		演示 2DC 子问题的一次成功判定：对于这种旋转和点集划分，用颜色标示了 K+(P) 、 K-(P) 的交，圆心在交集内就能完成覆盖，因此是成功判定

3. 预置数据
	1. Circle

		两个半径相同的圆上的点

	2. Rectangle

		与坐标轴对齐的矩形及其中垂线上的点

	3. Hyperbola

		一条抛物线和一条双曲线上的点

	4. Clear

		清空所有点
