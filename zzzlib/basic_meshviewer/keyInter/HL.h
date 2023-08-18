#ifndef _HL_H_
#define _HL_H_

#include<vector>
using namespace std;

#include"viewerKeyInter.h"
#include"viewerParas.h"
#include<queue>
#include<set>
#include<stack>
#include<algorithm>

namespace Viewer
{

    template<class T>
    class Compare {
    public:
        bool operator()(T v1, T v2)const
        {
            return v1->id() > v2->id();
        }
    };

    template <class T>
    class BinarySet
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
        set<T*,Compare<T*>> m_set;
    };

	template<typename M>
	class CHL :public CKeyInter
	{
	public:
		CHL(unsigned char _key, CTTMesh* _pMesh, CViewerParas* _pParas) :CKeyInter(_key)
		{
			m_pParas = _pParas;
			m_pMesh = _pMesh;
		}
		~CHL() {}

        set<CEdge*>   m_generators;

        virtual void keyInter(int _x, int _y)
        {

            for (M::MeshVertexIterator viter(m_pMesh); !viter.end(); viter++)
            {
                M::CVertex* pV = *viter;
                pV->pair() = NULL;
            }

            int eid = 0;

            for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
            {
                M::CEdge* pE = *eiter;
                pE->pair() = NULL;
                if (pE->boundary())
                    pE->id() = eid++;

                pE->setRGB(0, 0, 128.0 / 255.0);

            }

            for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
            {
                M::CEdge* pE = *eiter;
                if (!pE->boundary())pE->id() = eid++;

                pE->setRGB(0, 0, 128.0 / 255.0);
            }

            for (M::MeshFaceIterator fiter(m_pMesh); !fiter.end(); fiter++) {
                CFace* pF = *fiter;
                pF->highlight = false;
            }

            pair_surface();
            pair_interior();
        }

        void markHandleLoop(CFace* pF)
        {
            pF->highlight = true;
            BinarySet<CEdge> ecycle;

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
                std::cout << "head id = " << e->id() << std::endl;

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


        void pair_interior()
        {
            pairE(false);
            std::cout << "finished inner edges pair " << std::endl;
            pairF(false);
            std::cout << "finished inner faces pair " << std::endl;

            for (auto eiter = m_generators.begin(); eiter != m_generators.end(); eiter++)
            {
                CEdge* pE = *eiter;
                if (pE->pair() != NULL)
                {
                    CFace* pF = pE->pair();
                    markHandleLoop(pF);
                }
            }
        }

        void pair_surface()
        {
            pairE(true);
            std::cout << "finished boundary edges pair " << std::endl;
            pairF(true);
            std::cout << "finished boundary faces pair " << std::endl;

            for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
            {
                M::CEdge* pE = *eiter;
                if (pE->boundary() && pE->generator() && pE->pair() == NULL)
                {
                    m_generators.insert(pE);
                }
            }
        }

        /*
         * 采用过滤的方法
         * 给每个点分配一个边 
         * 由于CutGraph计算得是同伦群，有一个原点
         * 在生成树上计算
         * 而我们要计算的环柄圈不可能都经过一个点
         * 所以使用过滤的方法
         * 计算出一个森林
         */

        void pairE(bool isboundary)
        {

            for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
            {
                M::CEdge* pE = *eiter;
                if (pE->boundary() == isboundary) {
                    //std::cout << ".";
                    BinarySet<CVertex> vcycle;

                    CVertex* v = NULL;
                    CVertex* v1 = pE->vertex1();
                    CVertex* v2 = pE->vertex2();

                    vcycle.add(v1);
                    vcycle.add(v2);

                    v = vcycle.head();

                    while (v != NULL && v->pair() != NULL && !vcycle.empty()) {
                        CEdge* e = v->pair();
                        CVertex* v1 = pE->vertex1();
                        CVertex* v2 = pE->vertex2();

                        vcycle.add(v1);
                        vcycle.add(v2);

                        v = vcycle.head();
                    }

                    if (!vcycle.empty()) {
                        v->pair() = pE;
                        pE->setRGB(1, 1, 0);
                    }
                    else {
                        pE->generator() = true;// 标记为两个树之间的边
                    }
                }
            }
        }

        /*
         * 类比pairE
         *  给每一个边配上一个面，
         */
        void pairF(bool isboundary)
        {
            for (M::MeshFaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
            {
                M::CFace* pF = *fiter;
                if (pF->boundary() == isboundary) {
                    //std::cout << "-";
                    BinarySet<CEdge> ecycle;

                    for (M::HalfFaceHalfEdgeIterator feiter(m_pMesh, pF->left()); !feiter.end(); ++feiter) {
                        M::CHalfEdge* pE = *feiter;
                        ecycle.add(pE->tedge()->edge());
                    }

                    CEdge* e = NULL;
                    e = ecycle.head(); 
                    while (e != NULL && !e->generator()) {
                        ecycle.add(e);
                        e = ecycle.head();
                    }

                    //从最小的可挑选的边开始循环
                    while (e != NULL && e->pair() != NULL && !ecycle.empty()) {
                        CFace* f = e->pair();
                        for (M::HalfFaceHalfEdgeIterator feiter(m_pMesh, f->left()); !feiter.end(); ++feiter) {
                            M::CHalfEdge* pE = *feiter;
                            ecycle.add(pE->tedge()->edge());
                        }

                        e = ecycle.head();
                        while (e != NULL && !e->generator()) { 
                            ecycle.add(e);
                            e = ecycle.head();
                        }
                    }

                    if (!ecycle.empty()) {
                        e->pair() = pF;
                    }
                }
            }
        }


		CViewerParas* m_pParas;
		CTTMesh* m_pMesh;

	};

    template<typename M>
    class CVEHL :public CKeyInter
    {
    public:
        CVEHL(unsigned char _key, CTTMesh* _pMesh, CViewerParas* _pParas) :CKeyInter(_key)
        {
            m_pParas = _pParas;
            m_pMesh = _pMesh;
        }
        ~CVEHL() {}

        set<CEdge*>   m_generators;

        virtual void keyInter(int _x, int _y)
        {

            for (M::MeshVertexIterator viter(m_pMesh); !viter.end(); viter++)
            {
                M::CVertex* pV = *viter;
                pV->pair() = NULL;
            }

            int eid = 0;

            for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
            {
                M::CEdge* pE = *eiter;
                pE->pair() = NULL;
                if (pE->boundary())
                    pE->id() = eid++;

                pE->setRGB(0, 0, 128.0 / 255.0);

            }

            for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
            {
                M::CEdge* pE = *eiter;
                if (!pE->boundary())pE->id() = eid++;

                pE->setRGB(0, 0, 128.0 / 255.0);
            }

            for (M::MeshFaceIterator fiter(m_pMesh); !fiter.end(); fiter++) {
                CFace* pF = *fiter;
                pF->highlight = false;
            }

            pair_surface();
            pair_interior();
        }

        void markHandleLoop(CFace* pF)
        {
            pF->highlight = true;
            BinarySet<CEdge> ecycle;

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
                std::cout << "head id = " << e->id() << std::endl;

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


        void pair_interior()
        {
            pairE(false);
            std::cout << "finished inner edges pair " << std::endl;
            pairF(false);
            std::cout << "finished inner faces pair " << std::endl;

            for (auto eiter = m_generators.begin(); eiter != m_generators.end(); eiter++)
            {
                CEdge* pE = *eiter;
                if (pE->pair() != NULL)
                {
                    CFace* pF = pE->pair();
                    markHandleLoop(pF);
                }
            }
        }

        void pair_surface()
        {
            pairE(true);
            std::cout << "finished boundary edges pair " << std::endl;
            pairF(true);
            std::cout << "finished boundary faces pair " << std::endl;

            for (M::MeshEdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
            {
                M::CEdge* pE = *eiter;
                if (pE->boundary() && pE->generator() && pE->pair() == NULL)
                {
                    m_generators.insert(pE);
                }
            }
        }

        void pairE(bool isBoundary)
        {

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
        }

        /*
         * 类比pairE
         *  给每一个边配上一个面，
         */
        void pairF(bool isboundary)
        {
            for (M::MeshFaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
            {
                M::CFace* pF = *fiter;
                if (pF->boundary() == isboundary) {
                    //std::cout << "-";
                    BinarySet<CEdge> ecycle;

                    for (M::HalfFaceHalfEdgeIterator feiter(m_pMesh, pF->left()); !feiter.end(); ++feiter) {
                        M::CHalfEdge* pE = *feiter;
                        ecycle.add(pE->tedge()->edge());
                    }

                    CEdge* e = NULL;
                    e = ecycle.head();
                    while (e != NULL && !e->generator()) {
                        ecycle.add(e);
                        e = ecycle.head();
                    }

                    //从最小的可挑选的边开始循环
                    while (e != NULL && e->pair() != NULL && !ecycle.empty()) {
                        CFace* f = e->pair();
                        for (M::HalfFaceHalfEdgeIterator feiter(m_pMesh, f->left()); !feiter.end(); ++feiter) {
                            M::CHalfEdge* pE = *feiter;
                            ecycle.add(pE->tedge()->edge());
                        }

                        e = ecycle.head();
                        while (e != NULL && !e->generator()) {
                            ecycle.add(e);
                            e = ecycle.head();
                        }
                    }

                    if (!ecycle.empty()) {
                        e->pair() = pF;
                    }
                }
            }
        }


        CViewerParas* m_pParas;
        CTTMesh* m_pMesh;

    };
}

#endif
