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
#include <QThread>
#include <QtConcurrentRun>
#include <QReadWriteLock>

#include "kslideshow.h"

class KImageSlideShow : public KSlideShow<QImage>
{
	public:
		KImageSlideShow(QSize size = QSize(0,0), bool rand = false)
			: is_string_rand_(rand)
			, preload_(3)
			, steps_left_preload_(3)
			, current_size_(size)
		{
			is_random_ = false; // disable the rand provided by KSlideShow, we override it..
			filters_ << "*.jpeg" << "*.jpg" << "*.png" << "*.svg" << "*.svgz"; // use mime types?
		}

		void addDirectory(const QString& path, bool recrusive = false)
		{
			dir_ = path;
			recrusive_ = recrusive;
			QtConcurrent::run(this,&KImageSlideShow::addDirectoryThread);
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
				QtConcurrent::run(this,&KImageSlideShow::preloadForward);
				QtConcurrent::run(this,&KImageSlideShow::erasePreloadFromFront);
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
				QtConcurrent::run(this,&KImageSlideShow::preloadBackward);
				QtConcurrent::run(this,&KImageSlideShow::erasePreloadFromBack);
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
				lock_.lockForWrite();
				qSort(files_);
				lock_.unlock();
			}
		}

		void clear()
		{
			lock_.lockForWrite();
			KSlideShow<QImage>::clear();
			files_.clear();
			lock_.unlock();
		}

		void setSize(QSize size)
		{
			if(current_size_ != size) {
				// reload
				current_size_ = size;
			}
		}

	protected:
		void addDirectoryThread()
		{
			QString path = dir_;
			bool recrusive = recrusive_;

			bool set_ite = false;
			if( files_.isEmpty() ) set_ite = true;

			//Iterator pointing to the last item before the insertion of the new items.
			QStringList::iterator ite_last( files_.end()-1 );

			QDir dir(path);

			dir.setNameFilters(filters_);
			if (dir.entryList().isEmpty())  {
				return;
			}

			lock_.lockForWrite();
			foreach (const QString &imageFile, dir.entryList(QDir::Files)) {
				files_.append(QString(path + '/' + imageFile));
			}
			lock_.unlock();

			if(is_string_rand_) {
				lock_.lockForWrite();
				KRandomSequence randomSequence;
	
				randomSequence.randomize( files_ );
				lock_.unlock();
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
				QtConcurrent::run(this,&KImageSlideShow::preloadForward);
				preloadBackward();
				QtConcurrent::run(this,&KImageSlideShow::preloadBackward);
			}
		}

		void randomize()
		{
			lock_.lockForWrite();
			KRandomSequence randomSequence;
			randomSequence.randomize(files_);
			lock_.unlock();
		}

		void preloadForward()
		{
			lock_.lockForWrite();
			for(int i = 0; i < preload_+1 && files_ite_previous_ != files_ite_next_; ++i) {
				if( ++files_ite_next_ != files_.end() ) {
					addSlide(
						bool(current_size_ == QSize(0,0))
							? QImage( *files_ite_next_ )
							: QImage( *files_ite_next_ ).scaled(current_size_,Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
					);
				} else {
					files_ite_next_ = files_.begin();
					addSlide(
						bool(current_size_ == QSize(0,0))
							? QImage( *files_ite_next_ )
							: QImage( *files_ite_next_ ).scaled(current_size_,Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
					);
				}
			}
			lock_.unlock();
		}

		void preloadBackward()
		{
			lock_.lockForWrite();
			for(int i = 0; i < preload_+1 && files_ite_previous_ != files_ite_next_; ++i) {
				if( files_ite_previous_ != files_.begin() ) {
// 					addSlideFront( QImage( *--files_ite_previous_ ) );
					addSlideFront(
						bool(current_size_ == QSize(0,0))
							? QImage( *--files_ite_previous_ )
							: QImage( *--files_ite_previous_ ).scaled(current_size_,Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
					);
				} else {
					files_ite_previous_ = files_.end();
// 					addSlideFront( QImage( *--files_ite_previous_ ) );
					addSlideFront(
						bool(current_size_ == QSize(0,0))
							? QImage( *--files_ite_previous_ )
							: QImage( *--files_ite_previous_ ).scaled(current_size_,Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
					);
				}
			}
			lock_.unlock();
		}

		// Remove the preloaded items from the front
		void erasePreloadFromFront()
		{
			lock_.lockForWrite();
			// if the iterators collide we cant remove items, as they all should be preloaded
			if(files_ite_previous_ != files_ite_next_) {
				for(int i = 0; i < preload_+1; ++i) {
					++files_ite_previous_;
					slides_.pop_front();
					
				}
			}
			lock_.unlock();
		}

		// Remove the preloaded items fron the back
		void erasePreloadFromBack()
		{
			lock_.lockForWrite();
			// if the iterators collide we cant remove items, as they all should be preloaded
			if(files_ite_previous_ != files_ite_next_) {
				for(int i = 0; i < preload_+1; ++i) {
					--files_ite_next_;
					slides_.pop_back();

				}
			}
			lock_.unlock();
		}

	private:
		QStringList::iterator files_ite_next_;
		QStringList::iterator files_ite_previous_;
		QStringList files_;
		bool is_string_rand_;
		int preload_;
		QStringList filters_;
 		int steps_left_preload_;
		QString dir_;
		bool recrusive_;
		QReadWriteLock lock_;
		QSize current_size_;
};