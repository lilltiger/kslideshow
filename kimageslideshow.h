//
// C++ Interface: kimageslideshow
//
// Description: 
//
//
// Author: Peter Bengtsson <dev@lilltiger.se>, (C) 2008
//
// Copyright: GPLv3
//


#include <QDir>
#include "kslideshow.h"

class KImageSlideShow : public KSlideShow<QImage>
{
	public:
		KImageSlideShow(bool rand = false)
			: is_string_rand_(rand)
			, preload_(3)
			, steps_left_preload_(3)
		{
			is_random_ = false; // disable the rand provided by KSlideShow, we override it..
			filters_ << "*.jpeg" << "*.jpg" << "*.png" << "*.svg" << "*.svgz"; // use mime types?
		}

		void addDirectory(const QString& path, bool recrusive = false)
		{
			bool set_ite = false;
			if( files_.isEmpty() ) set_ite = true;

			//Iterator pointing to the last item before the insertion of the new items.
			QStringList::iterator ite_last( files_.end()-1 );

			QDir dir(path);

			dir.setNameFilters(filters_);
			if (dir.entryList().isEmpty())  {
				return;
			}

			foreach (const QString &imageFile, dir.entryList(QDir::Files)) {
				files_.append(QString(path + '/' + imageFile));
			}

			if(is_string_rand_) {
				KRandomSequence randomSequence;
	
				randomSequence.randomize( files_ );
				set_ite = true;
			} else {
				if( files_ite_previous_ == ite_last ) {
					files_ite_previous_ = files_.end()-1;
				}
				if( files_ite_next_ == files_.begin() ) {
					files_ite_next_ = ite_last+1;
				}
			}

			// First tme we added something or using a random list, set the iterators and preload.
			if(set_ite)
			{
				files_ite_next_ = files_.begin();
				files_ite_previous_ = files_.end()-1;
				// First time we preload we preload twice the normal amouth in both directions.
				preloadForward();
				preloadForward();
				preloadBackward();
				preloadBackward();
			}
		}

		void setPreloadAmouth(int preload)
		{
			steps_left_preload_ = preload;
			preload_ = preload;
		}

		QImage& next()
		{
			if( steps_left_preload_ == preload_*2 )
			{
				steps_left_preload_ = preload_;
				preloadForward();
				erasePreloadFromFront();
			} else {
				++steps_left_preload_;
			}
			return KSlideShow<QImage>::next();
		}

		QImage& previous()
		{
			if( steps_left_preload_ == 0 )
			{
				steps_left_preload_ = preload_;
				preloadBackward();
				erasePreloadFromBack();
			} else {
				--steps_left_preload_;
			}
			return KSlideShow<QImage>::previous();
		}

		// We are doing proloading so dont randomize the slides but randomize theset to preload from.
		void setRandom(bool rand)
		{
			is_string_rand_ = rand;
		
			if(rand) {
				randomize();
			} else {
				qSort(files_);
			}
		}

	protected:
		void randomize()
		{
			KRandomSequence randomSequence;
			randomSequence.randomize(files_);
		}

		void preloadForward()
		{
			for(int i = 0; i < preload_+1 && files_ite_previous_ != files_ite_next_; ++i) {
				if( ++files_ite_next_ != files_.end() ) {
					addSlide( QImage( *files_ite_next_ ) );
				} else {
					files_ite_next_ = files_.begin();
					addSlide( QImage( *files_ite_next_ ) );
				}
			}
		}

		void preloadBackward()
		{
			for(int i = 0; i < preload_+1 && files_ite_previous_ != files_ite_next_; ++i) {
				if( files_ite_previous_ != files_.begin() ) {
					addSlideFront( QImage( *--files_ite_previous_ ) );
				} else {
					files_ite_previous_ = files_.end();
					addSlideFront( QImage( *--files_ite_previous_ ) );
				}
			}
		}

		// Remove the preloaded items from the front
		void erasePreloadFromFront()
		{
			// if the iterators collide we cant remove items, as they all should be preloaded
			if(files_ite_previous_ != files_ite_next_) {
				for(int i = 0; i < preload_+1; ++i) {
					++files_ite_previous_;
					slides_.pop_front();
					
				}
			}
		}

		// Remove the preloaded items fron the back
		void erasePreloadFromBack()
		{
			// if the iterators collide we cant remove items, as they all should be preloaded
			if(files_ite_previous_ != files_ite_next_) {
				for(int i = 0; i < preload_+1; ++i) {
					--files_ite_next_;
					slides_.pop_back();

				}
			}
		}

	private:
		QStringList::iterator files_ite_next_;
		QStringList::iterator files_ite_previous_;
		QStringList files_;
		int preload_;
		bool is_string_rand_;
		QStringList filters_;
 		int steps_left_preload_;
};