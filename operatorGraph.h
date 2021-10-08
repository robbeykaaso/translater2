#include "rea.h"
#include <queue>

class operatorGraph : public std::enable_shared_from_this<operatorGraph>{
public:
    operatorGraph(const QString& aID = "");
    ~operatorGraph();
    void build(const QJsonObject& aConfig);
    void run(std::vector<QImage> aImages);
    void run();
private:
    QString m_name;
    std::vector<std::pair<int, QString>> m_starts;
    std::vector<QString> m_pipes;
};

//    {
//        "seq": ["id0", "id1"]
//    }

template <typename T, typename F>
class pipeMerge : public rea::pipe<T, F> {
public:
    pipeMerge(rea::pipeline* aParent, const QString& aName, int aThreadNo = 0) : rea::pipe<T, F>(aParent, aName, aThreadNo) {

    }
    rea::pipe0* initialize(F aFunc, const QJsonObject& aParam = QJsonObject()) override{
        auto seq = aParam.value("seq").toArray();
        for (auto i : seq)
            m_seqs.push_back(i.toString());
        return rea::pipe<T, F>::initialize(aFunc, aParam);
    }
protected:
    bool event( QEvent* e) override{
        if(e->type()== rea::pipe0::streamEvent::type){
            auto eve = reinterpret_cast<rea::pipe0::streamEvent*>(e);
            if (eve->getName() == rea::pipe0::m_name){
                auto stm = std::dynamic_pointer_cast<rea::stream<T>>(eve->getStream());
                auto scp = stm->scope();
                auto frm = scp->template data<QString>("from");
                auto imgs = scp->template data<std::vector<QImage>>("images");
                auto rets = scp->template data<std::vector<std::shared_ptr<rea::stream0>>>("results");
                if (m_cache.size() > 0){
                    bool cached = false;
                    for (auto i = m_cache.begin(); i != m_cache.end(); ++i)
                        if (!i->contains(frm)){
                            i->insert(frm, data(imgs, rets));
                            cached = true;
                        }
                    if (!cached){
                        m_cache.push_back(QHash<QString, data>());
                        m_cache.back().insert(frm, data(imgs, rets));
                    }
                    if (m_cache.front().size() == m_seqs.size()){
                        std::vector<QImage> cmb_imgs;
                        std::vector<std::shared_ptr<rea::stream0>> cmb_rets;
                        for (auto i : m_seqs){
                            auto dt = m_cache.front().value(i);
                            cmb_imgs.insert(cmb_imgs.end(), dt.images.begin(), dt.images.end());
                            cmb_rets.insert(cmb_rets.end(), dt.results.begin(), dt.results.end());
                        }
                        scp->template cache<std::vector<QImage>>("images", cmb_imgs);
                        scp->template cache<std::vector<std::shared_ptr<rea::stream0>>>("results", cmb_rets);
                        m_cache.pop_front();
                        doEvent(stm);
                        doNextEvent(rea::pipe0::m_next, stm);
                    }
                }else{
                    m_cache.push_back(QHash<QString, data>());
                    m_cache.back().insert(frm, data(imgs, rets));
                }
            }
        }
        return true;
    }
private:
    struct data{
        data(){}
        data(std::vector<QImage> aImages, std::vector<std::shared_ptr<rea::stream0>> aResults) : images(aImages), results(aResults){

        }
        std::vector<QImage> images;
        std::vector<std::shared_ptr<rea::stream0>> results;
    };
    std::vector<QString> m_seqs;
    std::deque<QHash<QString, data>> m_cache;
};
