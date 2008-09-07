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

#ifndef KSLIDESHOW_H
#define KSLIDESHOW_H

template<
	typename U
>
class KSlideShow
{
	public:
		KSlideShow() : is_random_(false)/*: ite_current_(slides_)*/ {}
		~KSlideShow()
		{
		}
		// Get previews, this dosent progress throught the slideshow
		U& previewNext()
		{
			return *ite_next_;
		}

		U& previewPrevious()
		{
			return *ite_previous_;
		}

		// Progress throught the slideshow
		U& next()
		{
			updateIterators(true);

			return *ite_current_;
		}

		U& previous()
		{
			updateIterators(false);

			return *ite_current_;
		}

		// Get the current item
		U& current()
		{
			return *ite_current_;
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
// 			U& item(*ite_current_);
			ite_current_ = slides_.erase(ite_current_);

			if( ite_current_ == slides_.end() ) {
				ite_current_ = slides_.begin();
			}

			if( ++ite_next_ == slides_.end() ) {
				ite_next_ = slides_.begin();
			}

//  			emit currentRemoved(/*item*/);
		}
		
		void clear()
		{
			slides_.clear();
		}
//  		signals:
//  			void currentRemoved(/*const U& removed*/);
	protected:
		void initIterators()
		{
			// If this was the first item to be added set the iterator to point to the front.
			if( slides_.count() == 1 ) {
				// Move to the first item
				ite_current_ = slides_.begin();
				ite_next_ = slides_.begin();
				ite_previous_ = slides_.begin();
			} else if( slides_.count() == 2 ) {
				// when 2 items in the list, both previous and next is the same
				ite_next_ = slides_.begin()+1;
				ite_previous_ = slides_.begin()+1;
			} else if( slides_.count() == 3 ) {
				// Now we can set the iterators as they should be
				ite_next_ = slides_.begin()+1;
				ite_previous_ = slides_.end()-1;
			}
		}

		void updateIterators(bool forward)
		{
			if(forward) {
				if( ite_current_ == slides_.end()-1 ) {
					// Current is the last item.
					ite_current_ = slides_.begin();
					ite_next_ = ite_current_+1;
					ite_previous_ = slides_.end()-1;
					
					return;
				} else {
					ite_previous_ = ite_current_++;

					if( ite_current_ == slides_.end()-1 ) {
						// Current (after increase) is the last visible item.
						ite_next_ = slides_.begin();
					} else {
						ite_next_ = ite_current_+1;
					}
					
					return;
				}
			} else { // backward
				if( ite_current_ == slides_.begin() ) {
					// Current is the first item.
					ite_current_ = slides_.end()-1;
					ite_next_ = slides_.begin();
					ite_previous_ = ite_current_-1;

					return;
				} else {
					ite_next_ = ite_current_--;

					if( ite_current_ == slides_.begin() ) {
					// Current (after the decrease) is the first item.
						ite_previous_ = slides_.end()-1;
					} else {
						ite_previous_ = ite_current_-1;
					}
				}
				
			}

		}

		void randomize()
		{
			KRandomSequence randomSequence;
			randomSequence.randomize(slides_);
		}

		bool is_random_;
		// Iterators that traverse the items
		typename QLinkedList<U>::iterator ite_current_;
		typename QLinkedList<U>::iterator ite_previous_;
		typename QLinkedList<U>::iterator ite_next_;

		QLinkedList<U> slides_;
};


#endif
