#ifndef BDN_TEST_TestViewCore_H_
#define BDN_TEST_TestViewCore_H_

#include <bdn/View.h>
#include <bdn/Window.h>
#include <bdn/test.h>
#include <bdn/IUiProvider.h>
#include <bdn/RequireNewAlloc.h>
#include <bdn/Button.h>

namespace bdn
{
namespace test
{


/** Helper for tests that verify IViewCore implementations.*/
class TestViewCore : public RequireNewAlloc<Base, TestViewCore>
{
public:

    /** Performs the tests.*/
    virtual void runTests()
    {
        _pWindow = newObj<Window>( &getUiProvider() );

        _pWindow->visible() = true;

        setView( createView() );

        // sanity check: the view should not have a parent yet
        REQUIRE( _pView->getParentView()==nullptr );

        SECTION("init")
        {      
            if( _pView == cast<View>(_pWindow) )
            {
                // the view is a window. These always have a core from
                // the start, so we cannot do any init tests with them.

                // only check that the view core is indeed already there.
                REQUIRE(_pView->getViewCore()!=nullptr );
            }
            else
            {
                // non-windows should not have a view core in the beginning
                // (before they are added to the window).

                REQUIRE(_pView->getViewCore()==nullptr );

                // run the init tests for them
                runInitTests();
            }
        }

        SECTION("postInit")
        {
            initCore();

            // view should always be visible for these tests
            _pView->visible() = true;

            // ensure that all pending initializations have finished.
            P<TestViewCore> pThis = this;
            CONTINUE_SECTION_WHEN_IDLE(pThis)
            {
                pThis->runPostInitTests();
            };
        }
    }    

protected:

    /** Returns true if the view position can be manually changed.
        Returns false if this is a UI element whose position is controlled
        by an external entity.
        
        The default implementation returns true
        */
    virtual bool canManuallyChangePosition() const
    {
        return true;
    }


    /** Returns true if the view sizecan be manually changed.
        Returns false if this is a UI element whose size is controlled
        by an external entity.
        
        The default implementation returns true
        */
    virtual bool canManuallyChangeSize() const
    {
        return true;
    }

    /** Runs the tests that verify that the core initializes itself with the current
        property values of the outer view.

        The core is not yet initialized when this function is called
        
        The tests each modify an outer view property, then cause the core to be created
        (by calling initCore()) and then verify that the core has initialized itself correctly.
        */
    virtual void runInitTests()
    {
        SECTION("visible")
        {
            _pView->visible() = true;

            initCore();
            verifyCoreVisibility();
        }
    
        SECTION("invisible")
        {
            _pView->visible() = false;

            initCore();
            verifyCoreVisibility();
        }

        SECTION("padding")
        {
            SECTION("default")
            {
                // the default padding of the outer view should be null (i.e. "use default").
                REQUIRE( _pView->padding().get().isNull() );

                initCore();
                verifyCorePadding();
            }

            SECTION("explicit")
            {
                _pView->padding() = UiMargin( UiLength::sem, 11, 22, 33, 44);

                initCore();
                verifyCorePadding();
            }
        }


        SECTION("bounds")
        {
            _pView->adjustAndSetBounds( Rect(110, 220, 880, 990) );

            initCore();
            verifyInitialDummyCorePosition();
            verifyInitialDummyCoreSize();
        }
    }

    /** Runs the tests that verify the core behaviour for operations that happen
        after the core is initialized.
        
        The core is already created/initialized when this is function is called.
        */
    virtual void runPostInitTests()
    {        
        SECTION("uiLengthToDips")
        {
            REQUIRE( _pCore->uiLengthToDips( UiLength(UiLength::dip, 0) ) == 0 );
            REQUIRE( _pCore->uiLengthToDips( UiLength(UiLength::sem, 0) ) == 0 );

            REQUIRE( _pCore->uiLengthToDips( UiLength(UiLength::dip, 17.34) ) == 17.34 );
            
            double semSize = _pCore->uiLengthToDips( UiLength(UiLength::sem, 1) );
            REQUIRE( semSize>0 );
            REQUIRE_ALMOST_EQUAL( _pCore->uiLengthToDips( UiLength(UiLength::sem, 3) ), semSize*3, 3);
        }

        SECTION("uiMarginToDipMargin")
        {
            SECTION("dip")
            {
                REQUIRE( _pCore->uiMarginToDipMargin( UiMargin(UiLength::dip, 10, 20, 30, 40) ) == Margin(10, 20, 30, 40) );
            }

            SECTION("sem")
            {
                double semDips = _pCore->uiLengthToDips( UiLength(UiLength::sem, 1) );

                Margin m = _pCore->uiMarginToDipMargin( UiMargin(UiLength::sem, 10, 20, 30, 40) );
                REQUIRE_ALMOST_EQUAL( m.top, 10*semDips, 10);
                REQUIRE_ALMOST_EQUAL( m.right, 20*semDips, 20);
                REQUIRE_ALMOST_EQUAL( m.bottom, 30*semDips, 30);
                REQUIRE_ALMOST_EQUAL( m.left, 40*semDips, 40);
            }
        }

        
        if(coreCanCalculatePreferredSize())
        {	
            SECTION("preferredSize")
            {
	            SECTION("calcPreferredSize plausible")	
                {
                    // we check elsewhere that padding is properly included in the preferred size
                    // So here we only check that the preferred size is "plausible"

                    Size prefSize = _pCore->calcPreferredSize();

                    REQUIRE( prefSize.width>=0 );
                    REQUIRE( prefSize.height>=0 );
                }
                
                SECTION("availableSize same as preferredSize")	
                {
                    SECTION("no padding")
                    {
                        // do nothing
                    }
                    
                    SECTION("with padding")
                    {
                        _pView->padding() = UiMargin( UiLength::Unit::dip, 10, 20, 30, 40);
                    }
                    
                    Size prefSize = _pCore->calcPreferredSize();
                    
                    Size prefSizeRestricted = _pCore->calcPreferredSize( prefSize.width, prefSize.height);
                    
                    REQUIRE( prefSize == prefSizeRestricted);
                }
                                
                SECTION("calcPreferredSize restrictedWidth plausible")	
                {
                    // this is difficult to test, since it depends heavily on what kind of view
                    // we actually work with. Also, it is perfectly normal for different core implementations
                    // to have different preferred size values for the same inputs.

                    // So we can only test rough plausibility here.
                    Size prefSize = _pCore->calcPreferredSize();

                    SECTION("unconditionalWidth")
                    {
                        // When we specify exactly the unconditional preferred width then we should get exactly the unconditional preferred height
                        REQUIRE( _pCore->calcPreferredSize(prefSize.width).height == prefSize.height );
                    }

                    SECTION("unconditionalWidth/2")
                    {
                        REQUIRE( _pCore->calcPreferredSize(prefSize.width/2).height >= prefSize.height );
                    }

                    SECTION("zero")
                    {
                        REQUIRE( _pCore->calcPreferredSize(0).height >= prefSize.height );
                    }
                }

                SECTION("calcPreferredSize restrictedHeight plausible")	
                {
                    Size prefSize = _pCore->calcPreferredSize();

                    SECTION("unconditionalHeight")
                        REQUIRE( _pCore->calcPreferredSize(-1, prefSize.height).width == prefSize.width );

                    SECTION("unconditionalHeight/2")
                        REQUIRE( _pCore->calcPreferredSize(-1, prefSize.height/2).width >= prefSize.width );
        
                    SECTION("zero")
                        REQUIRE( _pCore->calcPreferredSize(-1, 0).width >= prefSize.width );
                }
            }
        }
    
        SECTION("visibility")   
        {
            SECTION("visible")
            {
                _pView->visible() = true;
                verifyCoreVisibility();
            }

            SECTION("invisible")
            {
                _pView->visible() = false;
                verifyCoreVisibility();
            }

            if(coreCanCalculatePreferredSize())                
            {
                SECTION("noEffectOnPreferredSize")
                {
                    // verify that visibility has no effect on the preferred size
                    Size prefSizeBefore = _pCore->calcPreferredSize();

                    _pView->visible() = true;
                    REQUIRE( _pCore->calcPreferredSize() == prefSizeBefore );

                    _pView->visible() = false;
                    REQUIRE( _pCore->calcPreferredSize() == prefSizeBefore );

                    _pView->visible() = true;
                    REQUIRE( _pCore->calcPreferredSize() == prefSizeBefore );
                }
            }
        }

        SECTION("padding")
        {
            SECTION("custom")
            {
                _pView->padding() = UiMargin( UiLength::dip, 11, 22, 33, 44);
                verifyCorePadding();
            }

            SECTION("default after custom")
            {
                // set a non-default padding, then go back to default padding.
                _pView->padding() = UiMargin( UiLength::dip, 11, 22, 33, 44);
                _pView->padding() = nullptr;

                verifyCorePadding();
            }

            if(coreCanCalculatePreferredSize())
            {
                SECTION("effectsOnPreferredSize")
                {
                    // For some UI elements on some platforms there may be a silent minimum
                    // padding. If we specify a smaller padding then the minimum will be used
                    // instead.
        
                    // So to verify the effects of padding we first set a big padding that
                    // we are fairly confident to be over any minimum.
        
                    UiMargin paddingBefore(UiLength::sem, 10, 10, 10, 10);

                    _pView->padding() = paddingBefore;
                    
                    // wait a little so that sizing info is updated.
                    P<TestViewCore> pThis = this;
                    CONTINUE_SECTION_WHEN_IDLE( pThis, paddingBefore )
                    {        
                        Size prefSizeBefore = pThis->_pCore->calcPreferredSize();

                        UiMargin additionalPadding(UiLength::sem, 1, 2, 3, 4);
                        UiMargin increasedPadding = UiMargin(
                            UiLength::sem,
                            paddingBefore.top.value + additionalPadding.top.value,
                            paddingBefore.right.value + additionalPadding.right.value,
                            paddingBefore.bottom.value + additionalPadding.bottom.value,
                            paddingBefore.left.value + additionalPadding.left.value );

                        // setting padding should increase the preferred size
                        // of the core.
                        pThis->_pView->padding() = increasedPadding;


                        CONTINUE_SECTION_WHEN_IDLE( pThis, prefSizeBefore, additionalPadding )
                        {        
                            // the padding should increase the preferred size.
                            Size prefSize = pThis->_pCore->calcPreferredSize();

                            Margin  additionalPaddingPixels = pThis->_pView->uiMarginToDipMargin(additionalPadding);

                            REQUIRE_ALMOST_EQUAL( prefSize, prefSizeBefore+additionalPaddingPixels, Size(1,1) );
                        };
                    };
                }
            }
        }       

        
		SECTION("adjustAndSetBounds")
		{            
			Rect    bounds;
            Point   initialPosition = _pView->position();

            SECTION("no need to adjust")
            {
                // pre-adjust bounds so that we know that they are valid
                bounds = Rect(110, 220, 880, 990);
                bounds = _pCore->adjustBounds(bounds, RoundType::nearest, RoundType::nearest);
            }
            
            SECTION("need adjustments")
                bounds = Rect(110.12345, 220.12345, 880.12345, 990.12345);

            Rect returnedBounds = _pView->adjustAndSetBounds(bounds);            

            P<TestViewCore> pThis = this;
            
            // on some platform and with some view types (Linux / GTK with top level window)
            // waiting for idle is not enough to ensure that the position actually changed.
            // So instead we first wait for idle and then wait 2 additional seconds to ensure
            // that our changes have been applied
            CONTINUE_SECTION_WHEN_IDLE( pThis, bounds, returnedBounds )
            {
                CONTINUE_SECTION_AFTER_SECONDS(2, pThis, bounds, returnedBounds )
                {
                    // the core size and position should always represent what
                    // is configured in the view.
                    pThis->verifyCorePosition();
                    pThis->verifyCoreSize();

                    if(!pThis->canManuallyChangePosition())
                    {
                        // when the view cannot modify its position then trying to set another position should yield the same resulting position
                        Rect returnedBounds2 = pThis->_pCore->adjustAndSetBounds( Rect( bounds.x*2, bounds.y*2, bounds.width, bounds.height) );            
                        REQUIRE( returnedBounds2 == returnedBounds );

                        CONTINUE_SECTION_WHEN_IDLE( pThis )
                        {
                            // the core size and position should always represent what
                            // is configured in the view.
                            pThis->verifyCorePosition();
                            pThis->verifyCoreSize();
                        };
                    }
                };
            };
		}

        if(coreCanCalculatePreferredSize())
        {
            SECTION("adjustAndSetBounds no effect on preferred size")
            {
                Size prefSizeBefore = _pCore->calcPreferredSize();

                _pView->adjustAndSetBounds( Rect(110, 220, 880, 990) );
                    
                REQUIRE( _pCore->calcPreferredSize() == prefSizeBefore );
            }
        }


        SECTION("adjustBounds")
		{
            SECTION("no need to adjust")
            {
			    Rect bounds(110, 220, 880, 990);

                // pre-adjust the bounds
                bounds = _pCore->adjustBounds( bounds, RoundType::nearest, RoundType::nearest );

                std::list<RoundType> roundTypes{RoundType::nearest, RoundType::up, RoundType::down};

                for(RoundType positionRoundType: roundTypes)
                {
                    for(RoundType sizeRoundType: roundTypes)
                    {
                        SECTION( "positionRoundType: "+std::to_string((int)positionRoundType)+", "+std::to_string((int)sizeRoundType) )
                        {
                            Rect adjustedBounds = _pCore->adjustBounds(bounds, positionRoundType, sizeRoundType);

                            // no adjustments are necessary. So we should always get out the same that we put in
                            REQUIRE( adjustedBounds==bounds );
                        }
                    }
                }
            }

            SECTION("need adjustments")
            {
			    Rect bounds(110.12345, 220.12345, 880.12345, 990.12345);

                std::list<RoundType> roundTypes
                {
                    RoundType::down,
                    RoundType::nearest,
                    RoundType::up
                };

                std::vector<Rect> adjustedBoundsArray;

                for(RoundType sizeRoundType: roundTypes)
                {
                    for(RoundType positionRoundType: roundTypes)
                    {                    
                        Rect adjustedBounds = _pCore->adjustBounds(bounds, positionRoundType, sizeRoundType);

                        adjustedBoundsArray.push_back(adjustedBounds);
                    }
                }

                // there is not much we can do to "verify" the bounds. However, there are some relationships
                // of the rounding operations that must be true.

                // down-rounded must be <= nearest <=up

                for(int sizeRoundTypeIndex=0; sizeRoundTypeIndex<3; sizeRoundTypeIndex++)
                {
                    Point downRoundedPosition = adjustedBoundsArray[sizeRoundTypeIndex*3 + 0].getPosition();
                    Point nearestRoundedPosition = adjustedBoundsArray[sizeRoundTypeIndex*3 + 1].getPosition();
                    Point upRoundedPosition = adjustedBoundsArray[sizeRoundTypeIndex*3 + 2].getPosition();
                    
                    REQUIRE( downRoundedPosition <= nearestRoundedPosition );
                    REQUIRE( nearestRoundedPosition <= upRoundedPosition );

                    // if canManuallyChangePosition returns false then it means that the view
                    // has no control over its position. That means that adjustBounds will
                    // always return the view's current position.
                    if(canManuallyChangePosition())
                    {
                        REQUIRE( downRoundedPosition <= bounds.getPosition() );
                        REQUIRE( upRoundedPosition >= bounds.getPosition() );
                    }
                }

                for(int positionRoundTypeIndex=0; positionRoundTypeIndex<3; positionRoundTypeIndex++)
                {
                    Size downRoundedSize = adjustedBoundsArray[0*3 + positionRoundTypeIndex].getSize();
                    Size nearestRoundedSize = adjustedBoundsArray[1*3 + positionRoundTypeIndex].getSize();
                    Size upRoundedSize = adjustedBoundsArray[2*3 + positionRoundTypeIndex].getSize();
                    
                    REQUIRE( downRoundedSize <= nearestRoundedSize );
                    REQUIRE( nearestRoundedSize <= upRoundedSize );
                    
                    // if canManuallyChangeSize returns false then it means that the view
                    // has no control over its size. That means that adjustBounds will
                    // always return the view's current size.
                    if(canManuallyChangeSize())
                    {
                        REQUIRE( downRoundedSize <= bounds.getSize() );
                        REQUIRE( upRoundedSize >= bounds.getSize() );
                    }
                }
            }
        }        
    }


    /** Causes the core object to be created. This is done by adding the view as
        a child to a visible view container or window.*/
    virtual void initCore()
    {
        if(_pView!=cast<View>(_pWindow))
            _pWindow->setContentView( _pView );

        _pCore = _pView->getViewCore();

        REQUIRE( _pCore!=nullptr );
    }

    /** Verifies that the core's visible property matches that of the outer view.*/
    virtual void verifyCoreVisibility()=0;

    /** Verifies that the core's padding property matches that of the outer view.*/
    virtual void verifyCorePadding()=0;


    /** Verifies that the core's position property has the initial dummy value used
        directly after initialization.*/
    virtual void verifyInitialDummyCorePosition()=0;


    /** Verifies that the core's size property has the initial dummy value used
        directly after initialization.*/
    virtual void verifyInitialDummyCoreSize()=0;


    /** Verifies that the core's position property matches that of the outer view.*/
    virtual void verifyCorePosition()=0;

    /** Verifies that the core's size property matches that of the outer view.*/
    virtual void verifyCoreSize()=0;


    /** Returns the UiProvider to use.*/
    virtual IUiProvider& getUiProvider()=0;


    /** Creates the view object to use for the tests.*/
    virtual P<View> createView()=0;

    /** Sets the view object to use for the tests.*/
    virtual void setView(View* pView)
    {
        _pView = pView;
    }


    /** Returns true if the view core can calculate its preferred size.
        Some core types depend on the outer view to calculate the preferred size
        instead.
        
        The default implementation returns true.
        */
    virtual bool coreCanCalculatePreferredSize()
    {
        return true;
    }

    
    P<Window> _pWindow;
    P<View>   _pView;

    P<IViewCore> _pCore;
};



}
}

#endif

