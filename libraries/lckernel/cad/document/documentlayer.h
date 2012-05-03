#ifndef DOCUMENTLAYER_H
#define DOCUMENTLAYER_H

#include "cad/const.h"

#include "cad/meta/layer.h"
#include "cad/base/cadentity.h"
#include "cad/interface/variantable.h"

namespace lc {

    /**
     * Abstract class for a DocumentLayer
     *
     * it provides a interface to interact with the entities on the layer
     *
     */
    class DocumentLayer : public Variantable {

        public:
            /**
             * Add a entity to this layer
             * @param entity
             */
            virtual void addEntity(CADEntityPtr entity) = 0;

            /**
             * Remove a entity from this layer
             * @param id
             */
            virtual void removeEntity(ID_DATATYPE id) = 0;

            /**
             * Return a pointer to the layer meta object
             * @return
             */
            virtual LayerPtr layer() const = 0;

            /**
             * Return a list of all entities
             * @return
             */
            virtual QHash<int, CADEntityPtr>  const& allEntities() const = 0;

            /**
             * Return pointer to entity by ID
             * @return
             */
            virtual CADEntityPtr findByID(ID_DATATYPE) const = 0;
    };
    typedef shared_ptr<lc::DocumentLayer> DocumentLayerPtr;
}


#endif // DOCUMENTLAYER_H