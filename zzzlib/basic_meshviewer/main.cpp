#include<iostream>
using namespace std;
/* Mesh */
#include"TetTestMesh.h"
using namespace TMeshLib;
/* viewer */
#include"viewerCore/viewer.h"
#include"viewerDraw/viewerDraw.h"
#include"preprocess/viewerPrepares.h"
#include"keyInter/Resize_multi_08.h"
#include"keyInter/Resize_divid_08.h"
#include"keyInter/ShowEdgeInter.h"
#include"keyInter/BFS.h"
#include"keyInter/Ring.h"
#include"keyInter/HL.h"
#include"keyInter/CHL.h"
#include"keyInter/SetVisible.h"

using namespace Viewer;

/* main  */
int main(int argc, char * argv[])
{
	/* �����ļ� */
	CTTMesh mesh;
	mesh._load_t(argv[1]);
	/* Ԥ���� */
	PreOpers.addOper(new CPreNormal<CTTMesh>(&mesh));
	PreOpers.addOper(new CPreBoundary<CTTMesh>(&mesh));
	/* ���ӻ� */
	DrawOpers.addOper(new CDrawColoredEdges<CTTMesh>(&mesh,&mViewerParas));
	DrawOpers.addOper(new CDrawFaces<CTTMesh>(&mesh,&mViewerParas));
	/* ���̽��� */
	KeyBdInter.addKeyInter(new CKeyResizeMulti08<CTTMesh>('*',&mesh));
	KeyBdInter.addKeyInter(new CKeyResizeDivid08<CTTMesh>('/',&mesh));
	KeyBdInter.addKeyInter(new CShowEdgeInter<CTTMesh>('e',&mViewerParas));
	KeyBdInter.addKeyInter(new CSetVisible<CTTMesh>('v', &mesh, &mViewerParas));
	KeyBdInter.addKeyInter(new CBFS<CTTMesh>('b', &mesh, &mViewerParas));
	KeyBdInter.addKeyInter(new BFSFace<CTTMesh>('B', &mesh, &mViewerParas));
	KeyBdInter.addKeyInter(new CutFace<CTTMesh>('f', &mesh, &mViewerParas));
	KeyBdInter.addKeyInter(new CHandleLoop<CTTMesh>('c', &mesh, &mViewerParas));
	KeyBdInter.addKeyInter(new CRing<CTTMesh>('r', &mesh, &mViewerParas));
	KeyBdInter.addKeyInter(new CVEHL<CTTMesh>('H', &mesh, &mViewerParas));
	KeyBdInter.addKeyInter(new CHL<CTTMesh>('h', &mesh, &mViewerParas));
	/* OpenGL */
	viewer(argc, argv, NULL);
}