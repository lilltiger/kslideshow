//
// C++ Interface: kslideshow
//
// Description: 
//
//
// Author: Peter Bengtsson <dev@lilltiger.se>, (C) 2008
//
// Copyright: GPLv3
//

#include <QtAlgorithms>
#include <QtCore/QLinkedList>
#include <krandomsequence.h>
#include <krandom.h>
#include <QtCore/QObject>

#include "qcirculariterator.h"

#ifndef KSLIDESHOW_H
#define KSLIDESHOW_H



template<
	typename U
>
class KSlideShow
{
	public:
		KSlideShow() : is_random_(false), iterator_current_(slides_)/*: ite_current_(slides_)*/ {}
		~KSlideShow()
		{
		}
		// Get previews, this dosent progress throught the slideshow
		U& previewNext()
		{
			return *(iterator_current_+1);
		}

		U& previewPrevious()
		{
			return *(iterator_current_-1);
		}

		// Progress throught the slideshow
		U& next()
		{
			return *(++iterator_current_);
		}

		U& previous()
		{
			return *(--iterator_current_);
		}

		// Get the current item
		U& current()
		{
			return *iterator_current_;
		}

		bool isRandom() const { return is_random_; }

		void setRandom(bool rand)
		{
			is_random_ = rand;
		
			if(rand) {
				
				randomize();
			} else {
				qSort(slides_);
			}
		}

		/*typename QLinkedList<U>::iterator*/
		void addSlide(const U& slide)
		{
		
			typename QLinkedList<U>::iterator ite;
			if(is_random_ && slides_.count() > 1) {
				// modulus % is "broken" it will choose 80% in the upper halfe of the set.
				ite = slides_.insert(
					slides_.begin()+( (KRandom::random()%(slides_.count()-1)) ) // insert into an randomly choosed pos.
					,slide
				);
			} else {
				ite = slides_.insert(slides_.end(),slide);
			}

			initIterators();

// 			return ite;
		}

		void addSlideFront(const U& slide)
		{
			slides_.push_front(slide);
			initIterators();
		}

		void removeCurrent()
		{
			iterator_current_ = slides_.erase(iterator_current_.getIterator());
		}
		
		void clear()
		{
			slides_.clear();
		}

	protected:
		void initIterators()
		{
			// If this was the first item to be added set the iterator to point to the front.
			if( slides_.count() == 1 ) {
				// Move to the first item
				iterator_current_.toFront();
			}
		}

		void randomize()
		{
			KRandomSequence randomSequence;
			randomSequence.randomize(slides_);
		}

		bool is_random_;
		// Iterators that traverse the items

		CircularIterator< QLinkedList<U> > iterator_current_;
		QLinkedList<U> slides_;
};


#endif
