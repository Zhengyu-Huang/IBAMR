// Filename: IBInstrumentationSpec.h
// Created on 11 Jun 2007 by Boyce Griffith
//
// Copyright (c) 2002-2017, Boyce Griffith
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of The University of North Carolina nor the names of
//      its contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef included_IBAMR_IBInstrumentationSpec
#define included_IBAMR_IBInstrumentationSpec

/////////////////////////////// INCLUDES /////////////////////////////////////

#include <unistd.h>
#include <string>
#include <vector>

#include "ibtk/Streamable.h"
#include "ibtk/StreamableFactory.h"
#include "tbox/Pointer.h"

namespace SAMRAI
{
namespace hier
{
template <int DIM>
class IntVector;
} // namespace hier
namespace tbox
{
class AbstractStream;
} // namespace tbox
} // namespace SAMRAI

/////////////////////////////// CLASS DEFINITION /////////////////////////////

namespace IBAMR
{
/*!
 * \brief Class IBInstrumentationSpec encapsulates the data required to
 * initialize distributed internal flow meters and pressure gauges.
 */
class IBInstrumentationSpec : public IBTK::Streamable
{
public:
    /*!
     * \brief Register this class and its factory class with the singleton
     * IBTK::StreamableManager object.  This method must be called before any
     * IBInstrumentationSpec objects are created.
     *
     * \note This method is collective on all MPI processes.  This is done to
     * ensure that all processes employ the same class ID for the
     * IBInstrumentationSpec class.
     */
    static void registerWithStreamableManager();

    /*!
     * \brief Returns a boolean indicating whether the class has been registered
     * with the singleton IBTK::StreamableManager object.
     */
    static bool getIsRegisteredWithStreamableManager();

    /*!
     * The unique class ID for this object type assigned by the
     * IBTK::StreamableManager.
     */
    static int STREAMABLE_CLASS_ID;

    /*!
     * \brief Set the names of the flow meters and pressure gauges.
     */
    static void setInstrumentNames(const std::vector<std::string>& names);

    /*!
     * \brief Get the names of the flow meters and pressure gauges.
     */
    static const std::vector<std::string>& getInstrumentNames();

    /*!
     * \brief Default constructor.
     */
    IBInstrumentationSpec(int master_idx = -1, int meter_idx = -1, int node_idx = -1);

    /*!
     * \brief Destructor.
     */
    ~IBInstrumentationSpec();

    /*!
     * \return A const reference to the master node index.
     */
    const int& getMasterNodeIndex() const;

    /*!
     * \return A non-const reference to the master node index.
     */
    int& getMasterNodeIndex();

    /*!
     * \return A const reference to the meter index associated with the master
     * node.
     */
    const int& getMeterIndex() const;

    /*!
     * \return A non-const reference to the meter index associated with the
     * master node.
     */
    int& getMeterIndex();

    /*!
     * \return A const reference to the node index associated with the master
     * node.
     */
    const int& getNodeIndex() const;

    /*!
     * \return A non-const reference to the node index associated with the master
     * node.
     */
    int& getNodeIndex();

    /*!
     * \brief Return the unique identifier used to specify the
     * IBTK::StreamableFactory object used by the IBTK::StreamableManager to
     * extract Streamable objects from data streams.
     */
    int getStreamableClassID() const;

    /*!
     * \brief Return an upper bound on the amount of space required to pack the
     * object to a buffer.
     */
    size_t getDataStreamSize() const;

    /*!
     * \brief Pack data into the output stream.
     */
    void packStream(SAMRAI::tbox::AbstractStream& stream);

private:
    /*!
     * \brief Copy constructor.
     *
     * \note This constructor is not implemented and should not be used.
     *
     * \param from The value to copy to this object.
     */
    IBInstrumentationSpec(const IBInstrumentationSpec& from);

    /*!
     * \brief Assignment operator.
     *
     * \note This operator is not implemented and should not be used.
     *
     * \param that The value to assign to this object.
     *
     * \return A reference to this object.
     */
    IBInstrumentationSpec& operator=(const IBInstrumentationSpec& that);

    /*!
     * The names of the instrument names.
     */
    static std::vector<std::string> s_instrument_names;

    /*!
     * Data required to define the instrument.
     */
    int d_master_idx, d_meter_idx, d_node_idx;

    /*!
     * \brief A factory class to rebuild IBInstrumentationSpec objects from
     * SAMRAI::tbox::AbstractStream data streams.
     */
    class Factory : public IBTK::StreamableFactory
    {
    public:
        /*!
         * \brief Destructor.
         */
        ~Factory();

        /*!
         * \brief Return the unique identifier used to specify the
         * IBTK::StreamableFactory object used by the IBTK::StreamableManager to
         * extract IBInstrumentationSpec objects from data streams.
         */
        int getStreamableClassID() const;

        /*!
         * \brief Set the unique identifier used to specify the
         * IBTK::StreamableFactory object used by the IBTK::StreamableManager to
         * extract IBInstrumentationSpec objects from data streams.
         */
        void setStreamableClassID(int class_id);

        /*!
         * \brief Build an IBInstrumentationSpec object by unpacking data from
         * the data stream.
         */
        SAMRAI::tbox::Pointer<IBTK::Streamable> unpackStream(SAMRAI::tbox::AbstractStream& stream,
                                                             const SAMRAI::hier::IntVector<NDIM>& offset);

    private:
        /*!
         * \brief Default constructor.
         */
        Factory();

        /*!
         * \brief Copy constructor.
         *
         * \note This constructor is not implemented and should not be used.
         *
         * \param from The value to copy to this object.
         */
        Factory(const Factory& from);

        /*!
         * \brief Assignment operator.
         *
         * \note This operator is not implemented and should not be used.
         *
         * \param that The value to assign to this object.
         *
         * \return A reference to this object.
         */
        Factory& operator=(const Factory& that);

        friend class IBInstrumentationSpec;
    };
    typedef IBInstrumentationSpec::Factory IBInstrumentationSpecFactory;
};
} // namespace IBAMR

/////////////////////////////// INLINE ///////////////////////////////////////

#include "ibamr/private/IBInstrumentationSpec-inl.h" // IWYU pragma: keep

//////////////////////////////////////////////////////////////////////////////

#endif //#ifndef included_IBAMR_IBInstrumentationSpec
