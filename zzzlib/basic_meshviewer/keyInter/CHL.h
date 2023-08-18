#ifndef _CHL_H_
#define _CHL_H_

#include<vector>
using namespace std;

#include"viewerKeyInter.h"
#include"viewerParas.h"
#include<queue>
#include<set>
#include<algorithm>

namespace Viewer
{

	template<typename M>
	class BFSFace :public CKeyInter
	{
	public:
		BFSFace(unsigned char _key, CTTMesh* _pMesh, CViewerParas* _pParas) :CKeyInter(_key)
		{
			m_pParas = _pParas;
			m_pMesh = _pMesh;
		}
		~BFSFace() {}

		virtual void keyInter(int _x, int _y)
		{
			for (M::MeshFaceIterator iter(m_pMesh); !iter.end(); ++iter) {
				CFace* pF = *iter;
				pF->highlight = false;
				pF->vis = false;
			}
			for (M::MeshEdgeIterator iter(m_pMesh); !iter.end(); ++iter) {
				CEdge* pE = *iter;
				pE->prev = nullptr;
				pE->base = nullptr;
				pE->vis = false;
			}
			queue<CEdge*> que;
			vector<CFace*> faces;
			CEdge* root = *m_pMesh->edges().begin();
			root->vis = true;
			que.push(root);
			while (!que.empty()) {
				CEdge* top = que.front();
				que.pop();
				for (M::EdgeFaceIterator iter(m_pMesh, (M::CEdge*)top); !iter.end(); ++iter) {
					CFace* pF = *iter;
					if (!pF->vis) {
						for (M::HalfFaceHalfEdgeIterator eiter(m_pMesh, pF->left()); !eiter.end(); ++eiter) {
							CHalfEdge* pHE = *eiter;
							CEdge* desti = pHE->tedge()->edge();
							if (!desti->vis) {
								pF->vis = true;
								pF->highlight = true;
								pF->pair = top;
								desti->prev = top;
								desti->base = top->base;
								if (desti->base == nullptr)desti->base = root;
								if (desti->base == root)desti->base = top;
								que.push(desti);
								desti->vis = true;
							}
						}

						if (!pF->vis) {
							faces.push_back(pF);
						}
					}
				}
			}

			/*random_shuffle(faces.begin(), faces.end());
			CEdge* selected = nullptr;
			for (CFace* f : faces) {
				if(e->vertex1()->base != e->vertex2()->base){
					e->setRGB(1, 1, 0);
					CVertex* desti = e->vertex1();
					if (desti != root) {
						selected = desti;
						while (selected != nullptr && selected->prev != nullptr) {
							CEdge* edge = fromVV(selected, selected->prev);
							edge->setRGB(1, 1, 0);
							selected = selected->prev;
						}
					}
					desti = e->vertex2();
					if (desti != root) {
						selected = desti;
						while (selected != nullptr && selected->prev != nullptr) {
							CEdge* edge = fromVV(selected, selected->prev);
							edge->setRGB(1, 1, 0);
							selected = selected->prev;
						}
					}
					break;
				}
			}*/
			

		}
		CViewerParas* m_pParas;
		CTTMesh* m_pMesh;
	};

	template<typename M>
	class CutFace :public CKeyInter
	{
	public:
		CutFace(unsigned char _key, CTTMesh* _pMesh, CViewerParas* _pParas) :CKeyInter(_key)
		{
			m_pParas = _pParas;
			m_pMesh = _pMesh;
		}
		~CutFace() {}

		virtual void keyInter(int _x, int _y)
		{
			for (M::MeshFaceIterator iter(m_pMesh); !iter.end(); ++iter) {
				CFace* pF = *iter;
				pF->highlight = false;
				pF->pair = nullptr;
				pF->vis = false;
			}
			for (M::MeshEdgeIterator iter(m_pMesh); !iter.end(); ++iter) {
				CEdge* pE = *iter;
				pE->prev = nullptr;
				pE->base = nullptr;
				pE->vis = false;

				pE->setRGB(0, 0, 128.0 / 255.0);
			}
			queue<CEdge*> que;
			vector<CFace*> faces;
			CEdge* root = *m_pMesh->edges().begin();
			root->vis = true;
			que.push(root);
			while (!que.empty()) {
				CEdge* top = que.front();
				que.pop();
				for (M::EdgeFaceIterator iter(m_pMesh, (M::CEdge*)top); !iter.end(); ++iter) {
					CFace* pF = *iter;
					if (!pF->vis) {
						pF->pair = top;
						for (M::HalfFaceHalfEdgeIterator eiter(m_pMesh, pF->left()); !eiter.end(); ++eiter) {
							CHalfEdge* pHE = *eiter;
							CEdge* desti = pHE->tedge()->edge();
							if (!desti->vis) {
								pF->vis = true;
								desti->prev = top;
								desti->base = top->base;
								if (desti->base == nullptr)desti->base = root;
								if (desti->base == root)desti->base = top;
								que.push(desti);
								desti->vis = true;
							}
						}

						if (!pF->vis) {
							faces.push_back(pF);
						}
					}
				}
			}

			random_shuffle(faces.begin(), faces.end());
			CEdge* selected = nullptr;
			for (CFace* f : faces) {
				vector<CEdge*> te;
				for (M::HalfFaceHalfEdgeIterator eiter(m_pMesh, f->left()); !eiter.end(); ++eiter) {
					CHalfEdge* pHE = *eiter;
					if (pHE->tedge()->edge() != f->pair) {
						te.push_back(pHE->tedge()->edge());
					}
				}
				if(te.size() == 2)
					if (te[0]->base != te[1]->base) {
						f->highlight = true;
						CEdge* desti = te[0];
						if (desti != root) {
							selected = desti;
							while (selected != nullptr && selected->prev != nullptr) {
								CFace* face = fromEE(selected, selected->prev);
								face->highlight = true;
								selected = selected->prev;
							}
						}
						desti = te[1];
						if (desti != root) {
							selected = desti;
							while (selected != nullptr && selected->prev != nullptr) {
								CFace* face = fromEE(selected, selected->prev);
								face->highlight = true;
								selected = selected->prev;
							}
						}
						break;
					}
				
			}
		}

		CFace* fromEE(CEdge* e1, CEdge* e2) {
			for (M::EdgeFaceIterator iter(m_pMesh, (M::CEdge*)e1); !iter.end(); ++iter) {
				CFace* pF = *iter;
				for (M::HalfFaceHalfEdgeIterator eiter(m_pMesh, pF->left()); !eiter.end(); ++eiter) {
					CHalfEdge* pHE = *eiter;
					CEdge* desti = pHE->tedge()->edge();
					if (desti == e2) {
						return pF;
					}
				}
			}
			return nullptr;
		}
		CViewerParas* m_pParas;
		CTTMesh* m_pMesh;
	};

	template <class T>
	class Cycle
	{
	public:
		/*
		  找id最大的元素
		*/
		T* head()
		{
			T* phead = NULL;
			if (!m_set.empty())phead = *m_set.begin();
			return phead;
		}

		/*
		* 如果T不在环里，就加入
		* 如果T在环里，就删除
		*/
		void add(T* pw)
		{
			if (m_set.find(pw) != m_set.end())
				m_set.erase(pw);
			else
				m_set.insert(pw);
		};

		bool empty() {
			return m_set.empty();
		}

	protected:
		set<T*> m_set;
	};

	template<typename M>
	class CHandleLoop :public CKeyInter
	{
	public:
		CHandleLoop(unsigned char _key, CTTMesh* _pMesh, CViewerParas* _pParas) :CKeyInter(_key)
		{
			m_pParas = _pParas;
			m_pMesh = _pMesh;
		}
		~CHandleLoop() {}

		/*
		* 选取一个顶点进行四面体网格CutGraph
		*/
		CVertex* CVE(bool isBoundary) {
			
			queue<CVertex*> que;
			vector<CEdge*> edges;
			CVertex* root = *m_pMesh->vertices().begin();
			root->vis = true;
			que.push(root);
			while (!que.empty()) {
				CVertex* pV = que.front();
				que.pop();
				for (M::VertexEdgeIterator iter(m_pMesh, (M::CVertex*)pV); !iter.end(); ++iter) {
					CEdge* pE = *iter;
					if (!pE->vis && pE->boundary() == isBoundary) {
						CVertex* desti = pE->vertex1();
						if (!desti->vis) {
							pE->vis = true;
							pE->setRGB(1, 1, 0);
							pE->highlight = true;
							desti->prev = pV;
							desti->base = pV->base;
							if (desti->base == nullptr)desti->base = root;
							if (desti->base == root)desti->base = pV;
							que.push(desti);
							desti->vis = true;
						}
						desti = pE->vertex2();
						if (!desti->vis) {
							pE->vis = true;
							pE->setRGB(1, 1, 0);
							pE->highlight = true;
							desti->prev = pV;
							desti->base = pV->base;
							if (desti->base == nullptr)desti->base = root;
							if (desti->base == root)desti->base = pV;
							que.push(desti);
							desti->vis = true;
						}
						if (!pE->vis) {
							pE->generator() = true; // 标记
						}
					}
				}
			}
			return root;
		}

		void CEF(CEdge * root,bool isBoundary) {
			for (M::MeshEdgeIterator iter(m_pMesh); !iter.end(); ++iter) {
				CEdge* pE = *iter;
				pE->prev = nullptr;
				pE->base = nullptr;
				pE->pair() = nullptr;
				pE->vis = false;
			}
			queue<CEdge*> que;
			vector<CFace*> faces;
			root->vis = true;
			que.push(root);
			while (!que.empty()) {
				CEdge* top = que.front();
				que.pop();
				for (M::EdgeFaceIterator iter(m_pMesh, (M::CEdge*)top); !iter.end(); ++iter) {
					CFace* pF = *iter;
					if (!pF->vis && pF->boundary() == isBoundary) {
						pF->pair = top;
						for (M::HalfFaceHalfEdgeIterator eiter(m_pMesh, pF->left()); !eiter.end(); ++eiter) {
							CHalfEdge* pHE = *eiter;
							CEdge* desti = pHE->tedge()->edge();
							if (!desti->vis) {

								desti->pair() = pF;
								if (!desti->generator()) {
									pF->vis = true;
									pF->highlight = true;
									desti->prev = top;
									desti->base = top->base;
									if (desti->base == nullptr)desti->base = root;
									if (desti->base == root)desti->base = top;
									que.push(desti);
									desti->vis = true;
								}
							}
						}

						if (!pF->vis) {
							faces.push_back(pF);
						}
					}
				}
			}
		}

		virtual void keyInter(int _x, int _y)
		{
			for (M::MeshVertexIterator iter(m_pMesh); !iter.end(); ++iter) {
				CVertex* vert = *iter;
				vert->prev = nullptr;
				vert->base = nullptr;
				vert->vis = false;
			}
			for (M::MeshEdgeIterator iter(m_pMesh); !iter.end(); ++iter) {
				CEdge* pE = *iter;
				pE->prev = nullptr;
				pE->base = nullptr;
				pE->generator() = false;
				pE->highlight = false;
				pE->vis = false;
			}
			for (M::MeshFaceIterator iter(m_pMesh); !iter.end(); ++iter) {
				CFace* pF = *iter;
				pF->highlight = false;
				pF->pair = nullptr;
				pF->vis = false;
			}

			CVertex* rt = CVE(true);
			for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); ++eiter) {
				CEdge* edge = *eiter;
				if (!edge->generator()) {
					CEF(edge,true);
				}
			}
			
			rt = CVE(false);
			for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); ++eiter) {
				CEdge* edge = *eiter;
				if (!edge->generator()) {
					CEF(edge,false);
				}
			}

			for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); ++eiter)
			{
				CEdge* pE = *eiter;
				if (pE->pair() != nullptr)
				{
					cout << "edge" << endl;
					CFace* pF = pE->pair();
					cmarkHandleLoop(pF);
				}
			}
		}



		void cmarkHandleLoop(CFace* pF)
		{
			pF->highlight = true;
			Cycle<CEdge> ecycle;

			for (M::HalfFaceHalfEdgeIterator feiter(m_pMesh, (M::CHalfFace*)pF->left()); !feiter.end(); ++feiter) {
				M::CHalfEdge* pE = *feiter;
				ecycle.add(pE->tedge()->edge());
			}

			CEdge* e = NULL;
			e = ecycle.head();
			while (e != NULL && !e->generator()) { //不是两个树之间的边
				ecycle.add(e);
				e = ecycle.head();
			}

			while (e != NULL && e->pair() != NULL && !ecycle.empty()) { // 每条边都访问两次
				cout << "face" << endl;
				CFace* f = e->pair();
				f->highlight = true;
				for (M::HalfFaceHalfEdgeIterator feiter(m_pMesh, f->left()); !feiter.end(); ++feiter) { // 内部的每条边都与两个拓扑圆盘上的面相连接
					M::CHalfEdge* pE = *feiter;
					ecycle.add(pE->tedge()->edge());
				}

				e = ecycle.head();
				while (e != NULL && (!e->generator() || e->boundary())) {
					if (e->boundary()) {
						e->setRGB(1, 0, 0); // 标记外表面的边即为环柄圈
					}
					ecycle.add(e);
					e = ecycle.head();
				}
			}
		}

		CFace* fromEE(CEdge* e1, CEdge* e2) {
			for (M::EdgeFaceIterator iter(m_pMesh, (M::CEdge*)e1); !iter.end(); ++iter) {
				CFace* pF = *iter;
				for (M::HalfFaceHalfEdgeIterator eiter(m_pMesh, pF->left()); !eiter.end(); ++eiter) {
					CHalfEdge* pHE = *eiter;
					CEdge* desti = pHE->tedge()->edge();
					if (desti == e2) {
						return pF;
					}
				}
			}
			return nullptr;
		}

		CEdge* fromVV(CVertex* vert1, CVertex* vert2) {
			for (M::VertexEdgeIterator iter(m_pMesh, (M::CVertex*)vert1); !iter.end(); ++iter) {
				CEdge* pE = *iter;
				if (pE->boundary()) {
					CVertex* desti = pE->vertex1();
					if (desti == vert2) {
						return pE;
					}
					desti = pE->vertex2();
					if (desti == vert2) {
						return pE;
					}
				}
			}
			return nullptr;
		}
		CViewerParas* m_pParas;
		CTTMesh* m_pMesh;
	};
}






#endif
