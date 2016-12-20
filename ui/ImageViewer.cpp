/*
 * ImageViewer.cpp
 *
 *  Created on: Dec 15, 2016
 *      Author: linh
 */
#include <iostream>
#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <fstream>
#include <time.h>
#include <string>
#include <algorithm>
using namespace std;

#include <QtGui/QApplication>
#include <QtGui/QScrollBar>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPrintDialog>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>
#include <QtGui/QPainter>
#include <QtGui/QCloseEvent>
#include <QtCore/QSettings>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QMessageBox>
#include <QtCore/QDebug>
#include <QtGui/QIcon>
#include <QtGui/qwidget.h>

#include "../imageModel/Point.h"
#include "../imageModel/Line.h"
#include "../imageModel/Edge.h"
#include "../imageModel/Matrix.h"
#include "../io/Reader.h"
#include "../segmentation/Thresholds.h"
#include "../segmentation/Canny.h"
#include "../segmentation/Suzuki.h"
#include "../imageModel/Image.h"

#include "../histograms/ShapeHistogram.h"
#include "../pointInterest/Treatments.h"
#include "../pointInterest/Segmentation.h"
#include "../pointInterest/LandmarkDetection.h"
#include "../utils/ImageConvert.h"
#include "../utils/Drawing.h"

#include "ImageViewer.h"

void ImageViewer::createFileMenu()
{
	openAct = new QAction(QIcon("./resources/ico/open.png"), tr("&Open..."),
		this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	saveAct = new QAction(QIcon("./resources/ico/save.png"), tr("&Save"), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save the document to disk"));
	saveAct->setEnabled(false);
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	saveAsAct = new QAction(tr("Save &As..."), this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);
	saveAsAct->setStatusTip(tr("Save the document under a new name"));
	saveAsAct->setEnabled(false);
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	closeAct = new QAction(tr("&Close"), this);
	closeAct->setShortcut(tr("Ctrl+W"));
	closeAct->setStatusTip(tr("Close this window"));
	connect(closeAct, SIGNAL(triggered()), this, SLOT(close()));

	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

}
void ImageViewer::createViewMenu()
{
	zoomInAct = new QAction(QIcon("./resources/ico/1uparrow.png"),
		tr("Zoom &In (25%)"), this);
	zoomInAct->setShortcut(tr("Ctrl++"));
	zoomInAct->setEnabled(false);
	connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

	zoomOutAct = new QAction(QIcon("./resources/ico/1downarrow.png"),
		tr("Zoom &Out (25%)"), this);
	zoomOutAct->setShortcut(tr("Ctrl+-"));
	zoomOutAct->setEnabled(false);
	connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

	normalSizeAct = new QAction(tr("&Normal Size"), this);
	normalSizeAct->setShortcut(tr("Ctrl+N"));
	normalSizeAct->setEnabled(false);
	connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

	fitToWindowAct = new QAction(tr("&Fit to Window"), this);
	fitToWindowAct->setEnabled(false);
	fitToWindowAct->setCheckable(true);
	fitToWindowAct->setShortcut(tr("Ctrl+J"));
	connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

}
void ImageViewer::viewMenuUpdateActions()
{
	zoomInAct->setEnabled(!fitToWindowAct->isChecked());
	zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
	normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void ImageViewer::scaleImage(double factor)
{
	Q_ASSERT(imageLabel->pixmap());
	scaleFactor *= factor;
	imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

	adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
	adjustScrollBar(scrollArea->verticalScrollBar(), factor);

	zoomInAct->setEnabled(scaleFactor < 3.0);
	zoomOutAct->setEnabled(scaleFactor > 0.333);
}
void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
	scrollBar->setValue(
		int(
			factor * scrollBar->value()
				+ ((factor - 1) * scrollBar->pageStep() / 2)));
}

void ImageViewer::createHelpMenu()
{
	aboutAct = new QAction(tr("&About MAELab"), this);
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}
void ImageViewer::createSegmentationMenu()
{
	binaryThresholdAct = new QAction(tr("&Binary threshold"), this);
	binaryThresholdAct->setEnabled(false);
	connect(binaryThresholdAct, SIGNAL(triggered()), this, SLOT(binThreshold()));

	cannyAct = new QAction(tr("&Canny"), this);
	cannyAct->setEnabled(false);
	connect(cannyAct, SIGNAL(triggered()), this, SLOT(cannyAlgorithm()));
}
void ImageViewer::createLandmarksMenu()
{
	autoLandmarksAct = new QAction(tr("&Automatic Landmarks"), this);
	autoLandmarksAct->setEnabled(false);
	connect(autoLandmarksAct, SIGNAL(triggered()), this,
		SLOT(extractLandmarks()));
}
void ImageViewer::createActions()
{
	createFileMenu();
	createViewMenu();
	createHelpMenu();
	createSegmentationMenu();
	createLandmarksMenu();
}
void ImageViewer::createMenus()
{
	fileMenu = new QMenu(tr("&File"), this);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();
	fileMenu->addAction(closeAct);
	fileMenu->addAction(exitAct);

	viewMenu = new QMenu(tr("&View"), this);
	viewMenu->addAction(zoomInAct);
	viewMenu->addAction(zoomOutAct);
	viewMenu->addSeparator();
	viewMenu->addAction(normalSizeAct);
	viewMenu->addAction(fitToWindowAct);

	helpMenu = new QMenu(tr("&Helps"), this);
	helpMenu->addAction(aboutAct);

	segmentationMenu = new QMenu(tr("&Segmentation"), this);
	segmentationMenu->addAction(binaryThresholdAct);
	segmentationMenu->addAction(cannyAct);

	dominantPointMenu = new QMenu(tr("&Landmarks"), this);
	dominantPointMenu->addAction(autoLandmarksAct);

	// add menus to GUI
	menuBar()->addMenu(fileMenu);
	menuBar()->addMenu(viewMenu);
	menuBar()->addMenu(segmentationMenu);
	menuBar()->addMenu(dominantPointMenu);
	menuBar()->addMenu(helpMenu);
}
void ImageViewer::createToolBars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(openAct);
	fileToolBar->addAction(saveAct);

	viewToolBar = addToolBar(tr("View"));
	viewToolBar->addAction(zoomInAct);
	viewToolBar->addAction(zoomOutAct);
}
void ImageViewer::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}
void ImageViewer::activeFunction()
{
	openAct->setEnabled(true);
	saveAct->setEnabled(true);
	saveAsAct->setEnabled(true);

	zoomInAct->setEnabled(true);
	zoomOutAct->setEnabled(true);
	fitToWindowAct->setEnabled(true);
	normalSizeAct->setEnabled(true);

	binaryThresholdAct->setEnabled(true);
	cannyAct->setEnabled(true);

	autoLandmarksAct->setEnabled(true);

	viewMenuUpdateActions();
	if (!fitToWindowAct->isChecked())
		imageLabel->adjustSize();
}

ImageViewer::ImageViewer()
{
	imageLabel = new QLabel;
	imageLabel->setBackgroundRole(QPalette::Base);
	imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	imageLabel->setScaledContents(true);

	scrollArea = new QScrollArea;
	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidget(imageLabel);
	setCentralWidget(scrollArea);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	setWindowTitle(tr(".: MAELab :."));
	resize(900, 700);

	setWindowIcon(QIcon("./resources/ico/ip.ico"));
	//parameterPanel = NULL;

}

ImageViewer::~ImageViewer()
{

}
void ImageViewer::loadImage(QString fn)
{

	matImage = new Image(fn.toStdString());
	qImage.load(fn);

	imageLabel->setPixmap(QPixmap::fromImage(qImage));
	scaleFactor = 1.0;

	saveAsAct->setEnabled(true);
	activeFunction();

	this->fileName = fn;
	setWindowTitle(tr("Image Viewer - ") + fileName);
	statusBar()->showMessage(tr("File loaded"), 2000);
}
void ImageViewer::loadImage(Image *_matImage, QImage _qImage, QString tt)
{
	matImage = _matImage;
	qImage = _qImage;
	imageLabel->setPixmap(QPixmap::fromImage(qImage));
	scaleFactor = 1.0;

	saveAct->setEnabled(true);
	activeFunction();

	setWindowTitle(tr("Image Viewer - ") + tt);
	statusBar()->showMessage(tr("Finished"), 2000);
}
// =========================== Slots =======================================
void ImageViewer::about()
{
	QMessageBox::about(this, tr("About MAELab"),
		tr(
			"<p><b>MAELab</b> is a software in Computer Vision. It provides the function to "
				"segmentation and detection dominant points.</p>"));
}
void ImageViewer::open()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
		QDir::currentPath());
	if (!fileName.isEmpty())
	{
		QImage image(fileName);
		if (image.isNull())
		{
			QMessageBox::information(this, tr("MAELab"),
				tr("Cannot load %1.").arg(fileName));
			return;
		}
		if (!this->fileName.isEmpty())
		{
			ImageViewer* other = new ImageViewer;
			other->loadImage(fileName);
			other->move(x() + 40, y() + 40);
			other->show();
		}
		else
		{
			this->loadImage(fileName);
		}
	}
}
void ImageViewer::save()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), ".",
		tr("Image Files (*.jpg)"));
	if (fileName.isEmpty())
	{
		cout << "\nCan not save the image !!";
		return;
	}
	QApplication::setOverrideCursor(Qt::WaitCursor);
	qImage.save(fileName, "JPEG");

	QApplication::restoreOverrideCursor();

	this->fileName = fileName;
	setWindowTitle(tr("MAELab - ") + fileName);

	saveAct->setEnabled(false);
	saveAsAct->setEnabled(true);

	statusBar()->showMessage(tr("File saved"), 2000);
	return;
}
void ImageViewer::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), ".",
		tr("Image Files (*.jpg)"));
	if (fileName.isEmpty())
	{
		cout << "\nCan not save the image !!";
		return;
	}
	QApplication::setOverrideCursor(Qt::WaitCursor);
	qImage.save(fileName, "JPEG");
	QApplication::restoreOverrideCursor();

	saveAct->setEnabled(true);

	statusBar()->showMessage(tr("File saved"), 2000);
	return;
}
void ImageViewer::zoomIn()
{
	scaleImage(1.25);
}
void ImageViewer::zoomOut()
{
	scaleImage(0.8);
}
void ImageViewer::normalSize()
{
	imageLabel->adjustSize();
	scaleFactor = 1.0;
}
void ImageViewer::fitToWindow()
{

	bool fitToWindow = fitToWindowAct->isChecked();
	scrollArea->setWidgetResizable(fitToWindow);
	if (!fitToWindow)
	{
		normalSize();
	}
	viewMenuUpdateActions();
}

void ImageViewer::binThreshold()
{
	cout << "\nBinary thresholding...\n";
	float tValue = matImage->getThresholdValue();
	Segmentation tr; // = new Segmentation();
	tr.setRefImage(*matImage);
	ptr_IntMatrix rsMatrix = tr.threshold(tValue, 255); //binaryThreshold(matImage->getGrayMatrix(), tValue,
	//255);

	ImageViewer *other = new ImageViewer;
	other->loadImage(matImage, ptrIntToQImage(rsMatrix), "Thresholding result");
	other->show();

}
void ImageViewer::cannyAlgorithm()
{
	cout << "\nCanny Algorithm...\n";
	Segmentation tr;
	tr.setRefImage(*matImage);
	vector<ptr_Edge> edges = tr.canny();

	RGB color;
	color.R = 255;
	color.G = color.B = 0;
	for (size_t i = 0; i < edges.size(); i++)
	{
		ptr_Edge edgei = edges.at(i);
		for (int k = 0; k < edgei->getPoints().size(); k++)
		{
			ptr_Point pi = edgei->getPoints().at(k);
			matImage->getRGBMatrix()->setAtPosition(pi->getY(), pi->getX(), color);
		}
	}
	ImageViewer *other = new ImageViewer;
	other->loadImage(matImage, ptrRGBToQImage(matImage->getRGBMatrix()),
		"Canny result");
	other->move(x() - 40, y() - 40);
	other->show();
}
void ImageViewer::extractLandmarks()
{
	cout << "\n Automatic extraction the landmarks.\n";
	QMessageBox msgbox;
	msgbox.setText("Select the landmark file of model image.");
	msgbox.exec();

	QString reflmPath = QFileDialog::getOpenFileName(this);
	matImage->readManualLandmarks(reflmPath.toStdString());

	LandmarkDetection tr;
	tr.setRefImage(*matImage);

	msgbox.setText("Select the scene image.");
	msgbox.exec();

	QString fileName2 = QFileDialog::getOpenFileName(this);
	if (fileName2.isEmpty())
		return;
	qDebug() << fileName2;

	Image *sceneImage = new Image(fileName2.toStdString());
	ptr_Point ePoint;
	double angleDiff;
	vector<ptr_Point> lms = tr.landmarksAutoDectect(*sceneImage, Degree, 500, 400,
		500, ePoint, angleDiff);
	cout << "\nNumber of the landmarks: " << lms.size();
	RGB color;
	color.R = 255;
	color.G = 255;
	color.B = 0;

	sceneImage->rotate(ePoint, angleDiff, 1);
	for (int i = 0; i < lms.size(); i++)
	{
		ptr_Point lm = lms.at(i);
		vector<ptr_Point> dPoints = drawingCircle(lm, 5, color);
		for (int k = 0; k < dPoints.size(); k++)
		{
			ptr_Point p = dPoints.at(k);
			sceneImage->getRGBMatrix()->setAtPosition(p->getY(), p->getX(),
				p->getColor());
		}
	}

	ImageViewer *other = new ImageViewer;
	other->loadImage(sceneImage, ptrRGBToQImage(sceneImage->getRGBMatrix()),
		"Landmarks result");
	other->move(x() - 40, y() - 40);
	other->show();
	cout << "\nFinish.\n";
}