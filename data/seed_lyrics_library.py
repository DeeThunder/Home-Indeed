"""
Seed script to expand Home Indeed worship lyrics library to 125+ standard church hymns & gospel classics.
Formatted with clear stanza paragraphs (\n\n) for automatic slide progression in OBS.
"""
import sqlite3
import re

DB_PATH = "data/homein-lyrics.db"

def normalize_key(title, artist):
    raw = f"{title}|{artist or ''}".lower()
    return re.sub(r'[^a-z0-9|]', '', raw)

SONGS = [
    # ==================== TRADITIONAL / CLASSIC HYMNS ====================
    (
        "How Great Thou Art",
        "Carl Boberg",
        """O Lord my God, when I in awesome wonder
Consider all the worlds Thy hands have made,
I see the stars, I hear the rolling thunder,
Thy power throughout the universe displayed.

Then sings my soul, my Savior God, to Thee:
How great Thou art, how great Thou art!
Then sings my soul, my Savior God, to Thee:
How great Thou art, how great Thou art!

When through the woods and forest glades I wander,
And hear the birds sing sweetly in the trees;
When I look down from lofty mountain grandeur
And hear the brook and feel the gentle breeze.

And when I think that God, His Son not sparing,
Sent Him to die, I scarce can take it in;
That on the cross, my burden gladly bearing,
He bled and died to take away my sin.

When Christ shall come with shout of acclamation
And take me home, what joy shall fill my heart!
Then I shall bow in humble adoration,
And there proclaim: "My God, how great Thou art!\""""
    ),
    (
        "Great Is Thy Faithfulness",
        "Thomas Chisholm",
        """Great is Thy faithfulness, O God my Father;
There is no shadow of turning with Thee;
Thou changest not, Thy compassions, they fail not;
As Thou hast been Thou forever wilt be.

Great is Thy faithfulness!
Great is Thy faithfulness!
Morning by morning new mercies I see;
All I have needed Thy hand hath provided;
Great is Thy faithfulness, Lord, unto me!

Summer and winter, and springtime and harvest,
Sun, moon and stars in their courses above,
Join with all nature in manifold witness
To Thy great faithfulness, mercy and love.

Pardon for sin and a peace that endureth,
Thine own dear presence to cheer and to guide;
Strength for today and bright hope for tomorrow,
Blessings all mine, with ten thousand beside!"""
    ),
    (
        "It Is Well With My Soul",
        "Horatio Spafford",
        """When peace, like a river, attendeth my way,
When sorrows like sea billows roll;
Whatever my lot, Thou hast taught me to say,
It is well, it is well with my soul.

It is well with my soul,
It is well, it is well with my soul.

Though Satan should buffet, though trials should come,
Let this blest assurance control,
That Christ has regarded my helpless estate,
And hath shed His own blood for my soul.

My sin, oh, the bliss of this glorious thought!
My sin, not in part but the whole,
Is nailed to the cross, and I bear it no more,
Praise the Lord, praise the Lord, O my soul!

And Lord, haste the day when my faith shall be sight,
The clouds be rolled back as a scroll;
The trump shall resound, and the Lord shall descend,
Even so, it is well with my soul."""
    ),
    (
        "Holy, Holy, Holy",
        "Reginald Heber",
        """Holy, holy, holy! Lord God Almighty!
Early in the morning our song shall rise to Thee;
Holy, holy, holy! Merciful and mighty!
God in three Persons, blessed Trinity!

Holy, holy, holy! All the saints adore Thee,
Casting down their golden crowns around the glassy sea;
Cherubim and seraphim falling down before Thee,
Which wert and art and evermore shalt be.

Holy, holy, holy! Though the darkness hide Thee,
Though the eye of sinful man Thy glory may not see;
Only Thou art holy; there is none beside Thee,
Perfect in power, in love, and purity.

Holy, holy, holy! Lord God Almighty!
All Thy works shall praise Thy Name in earth and sky and sea;
Holy, holy, holy! Merciful and mighty!
God in three Persons, blessed Trinity!"""
    ),
    (
        "Blessed Assurance",
        "Fanny Crosby",
        """Blessed assurance, Jesus is mine!
O what a foretaste of glory divine!
Heir of salvation, purchase of God,
Born of His Spirit, washed in His blood.

This is my story, this is my song,
Praising my Savior all the day long;
This is my story, this is my song,
Praising my Savior all the day long.

Perfect submission, perfect delight,
Visions of rapture now burst on my sight;
Angels descending bring from above
Echoes of mercy, whispers of love.

Perfect submission, all is at rest;
I in my Savior am happy and blest,
Watching and waiting, looking above,
Filled with His goodness, lost in His love."""
    ),
    (
        "What a Friend We Have in Jesus",
        "Joseph Scriven",
        """What a friend we have in Jesus,
All our sins and griefs to bear!
What a privilege to carry
Everything to God in prayer!
O what peace we often forfeit,
O what needless pain we bear,
All because we do not carry
Everything to God in prayer.

Have we trials and temptations?
Is there trouble anywhere?
We should never be discouraged;
Take it to the Lord in prayer.
Can we find a friend so faithful
Who will all our sorrows share?
Jesus knows our every weakness;
Take it to the Lord in prayer.

Are we weak and heavy laden,
Cumbered with a load of care?
Precious Savior, still our refuge;
Take it to the Lord in prayer.
Do thy friends despise, forsake thee?
Take it to the Lord in prayer!
In His arms He'll take and shield thee;
Thou wilt find a solace there."""
    ),
    (
        "Rock of Ages",
        "Augustus Toplady",
        """Rock of Ages, cleft for me,
Let me hide myself in Thee;
Let the water and the blood,
From Thy wounded side which flowed,
Be of sin the double cure,
Save from wrath and make me pure.

Not the labor of my hands
Can fulfill Thy law's demands;
Could my zeal no respite know,
Could my tears forever flow,
All for sin could not atone;
Thou must save, and Thou alone.

Nothing in my hand I bring,
Simply to the cross I cling;
Naked, come to Thee for dress;
Helpless, look to Thee for grace;
Foul, I to the fountain fly;
Wash me, Savior, or I die.

While I draw this fleeting breath,
When mine eyes shall close in death,
When I soar to worlds unknown,
See Thee on Thy judgment throne,
Rock of Ages, cleft for me,
Let me hide myself in Thee."""
    ),
    (
        "Come Thou Fount of Every Blessing",
        "Robert Robinson",
        """Come, Thou Fount of every blessing,
Tune my heart to sing Thy grace;
Streams of mercy, never ceasing,
Call for songs of loudest praise.
Teach me some melodious sonnet,
Sung by flaming tongues above;
Praise the mount! I'm fixed upon it,
Mount of Thy redeeming love.

Here I raise my Ebenezer;
Hither by Thy help I'm come;
And I hope, by Thy good pleasure,
Safely to arrive at home.
Jesus sought me when a stranger,
Wandering from the fold of God;
He, to rescue me from danger,
Interposed His precious blood.

O to grace how great a debtor
Daily I'm constrained to be!
Let Thy goodness, like a fetter,
Bind my wandering heart to Thee.
Prone to wander, Lord, I feel it,
Prone to leave the God I love;
Here's my heart, O take and seal it,
Seal it for Thy courts above."""
    ),
    (
        "All Hail the Power of Jesus' Name",
        "Edward Perronet",
        """All hail the power of Jesus' name!
Let angels prostrate fall;
Bring forth the royal diadem,
And crown Him Lord of all.
Bring forth the royal diadem,
And crown Him Lord of all.

Ye chosen seed of Israel's race,
Ye ransomed from the fall,
Hail Him who saves you by His grace,
And crown Him Lord of all.
Hail Him who saves you by His grace,
And crown Him Lord of all.

Let every kindred, every tribe
On this terrestrial ball,
To Him all majesty ascribe,
And crown Him Lord of all.
To Him all majesty ascribe,
And crown Him Lord of all.

O that with yonder sacred throng
We at His feet may fall!
We'll join the everlasting song,
And crown Him Lord of all.
We'll join the everlasting song,
And crown Him Lord of all."""
    ),
    (
        "Be Thou My Vision",
        "Dallan Forgaill / Eleanor Hull",
        """Be Thou my Vision, O Lord of my heart;
Naught be all else to me, save that Thou art;
Thou my best thought, by day or by night,
Waking or sleeping, Thy presence my light.

Be Thou my Wisdom, and Thou my true Word;
I ever with Thee and Thou with me, Lord;
Thou my great Father, I Thy true son;
Thou in me dwelling, and I with Thee one.

Riches I heed not, nor man's empty praise,
Thou mine inheritance, now and always;
Thou and Thou only, first in my heart,
High King of Heaven, my Treasure Thou art.

High King of Heaven, my victory won,
May I reach Heaven's joys, O bright Heaven's Sun!
Heart of my own heart, whatever befall,
Still be my Vision, O Ruler of all."""
    ),
    (
        "The Old Rugged Cross",
        "George Bennard",
        """On a hill far away stood an old rugged cross,
The emblem of suffering and shame;
And I love that old cross where the dearest and best
For a world of lost sinners was slain.

So I'll cherish the old rugged cross,
Till my trophies at last I lay down;
I will cling to the old rugged cross,
And exchange it some day for a crown.

O that old rugged cross, so despised by the world,
Has a wondrous attraction for me;
For the dear Lamb of God left His glory above
To bear it to dark Calvary.

In the old rugged cross, stained with blood so divine,
A wondrous beauty I see,
For 'twas on that old cross Jesus suffered and died,
To pardon and sanctify me.

To the old rugged cross I will ever be true,
Its shame and reproach gladly bear;
Then He'll call me some day to my home far away,
Where His glory forever I'll share."""
    ),
    (
        "Crown Him With Many Crowns",
        "Matthew Bridges",
        """Crown Him with many crowns,
The Lamb upon His throne;
Hark! How the heavenly anthem drowns
All music but its own!
Awake, my soul, and sing
Of Him who died for thee,
And hail Him as thy matchless King
Through all eternity.

Crown Him the Lord of life,
Who triumphed o'er the grave,
And rose victorious in the strife
For those He came to save.
His glories now we sing,
Who died, and rose on high,
Who came eternal life to bring,
And lives that death may die.

Crown Him the Lord of peace,
Whose power a scepter sways
From pole to pole, that wars may cease,
And all be prayer and praise.
His reign shall know no end,
And round His pierced feet
Fair flowers of paradise extend
Their fragrance ever sweet.

Crown Him the Lord of love,
Behold His hands and side,
Those wounds, yet visible above,
In beauty glorified.
All hail, Redeemer, hail!
For Thou hast died for me;
Thy praise and glory shall not fail
Throughout eternity."""
    ),
    (
        "To God Be the Glory",
        "Fanny Crosby",
        """To God be the glory, great things He hath done!
So loved He the world that He gave us His Son,
Who yielded His life an atonement for sin,
And opened the life gate that all may go in.

Praise the Lord, praise the Lord,
Let the earth hear His voice!
Praise the Lord, praise the Lord,
Let the people rejoice!
O come to the Father, through Jesus the Son,
And give Him the glory, great things He hath done.

O perfect redemption, the purchase of blood,
To every believer the promise of God;
The vilest offender who truly believes,
That moment from Jesus a pardon receives.

Great things He hath taught us, great things He hath done,
And great our rejoicing through Jesus the Son;
But purer, and higher, and greater will be
Our wonder, our transport, when Jesus we see."""
    ),
    (
        "'Tis So Sweet to Trust in Jesus",
        "Louisa Stead",
        """'Tis so sweet to trust in Jesus,
Just to take Him at His word;
Just to rest upon His promise;
Just to know, "Thus saith the Lord."

Jesus, Jesus, how I trust Him!
How I've proved Him o'er and o'er!
Jesus, Jesus, precious Jesus!
O for grace to trust Him more!

O how sweet to trust in Jesus,
Just to trust His cleansing blood;
Just in simple faith to plunge me
'Neath the healing, cleansing flood!

Yes, 'tis sweet to trust in Jesus,
Just from sin and self to cease;
Just from Jesus simply taking
Life and rest, and joy and peace.

I'm so glad I learned to trust Thee,
Precious Jesus, Savior, Friend;
And I know that Thou art with me,
Wilt be with me to the end."""
    ),
    (
        "Nearer, My God, to Thee",
        "Sarah Adams",
        """Nearer, my God, to Thee, nearer to Thee!
E'en though it be a cross that raiseth me,
Still all my song shall be,
Nearer, my God, to Thee,
Nearer, my God, to Thee, nearer to Thee!

Though like the wanderer, the sun gone down,
Darkness be over me, my rest a stone;
Yet in my dreams I'd be
Nearer, my God, to Thee,
Nearer, my God, to Thee, nearer to Thee!

There let the way appear steps unto heaven;
All that Thou sendest me in mercy given;
Angels to beckon me
Nearer, my God, to Thee,
Nearer, my God, to Thee, nearer to Thee!

Then, with my waking thoughts bright with Thy praise,
Out of my stony griefs Bethel I'll raise;
So by my woes to be
Nearer, my God, to Thee,
Nearer, my God, to Thee, nearer to Thee!"""
    ),
    (
        "Standing on the Promises",
        "Russell Kelso Carter",
        """Standing on the promises of Christ my King,
Through eternal ages let His praises ring;
Glory in the highest, I will shout and sing,
Standing on the promises of God.

Standing, standing,
Standing on the promises of God my Savior;
Standing, standing,
I'm standing on the promises of God.

Standing on the promises that cannot fail,
When the howling storms of doubt and fear assail,
By the living Word of God I shall prevail,
Standing on the promises of God.

Standing on the promises of Christ the Lord,
Bound to Him eternally by love's strong cord,
Overcoming daily with the Spirit's sword,
Standing on the promises of God.

Standing on the promises I cannot fall,
Listening every moment to the Spirit's call,
Resting in my Savior as my all in all,
Standing on the promises of God."""
    ),
    (
        "Joyful, Joyful We Adore Thee",
        "Henry van Dyke",
        """Joyful, joyful, we adore Thee,
God of glory, Lord of love;
Hearts unfold like flowers before Thee,
Opening to the sun above.
Melt the clouds of sin and sadness;
Drive the dark of doubt away;
Giver of immortal gladness,
Fill us with the light of day!

All Thy works with joy surround Thee,
Earth and heaven reflect Thy rays,
Stars and angels sing around Thee,
Center of unbroken praise.
Field and forest, vale and mountain,
Flowery meadow, flashing sea,
Chanting bird and flowing fountain
Call us to rejoice in Thee.

Thou art giving and forgiving,
Ever blessing, ever blest,
Wellspring of the joy of living,
Ocean depth of happy rest!
Thou our Father, Christ our Brother,
All who live in love are Thine;
Teach us how to love each other,
Lift us to the joy divine."""
    ),
    (
        "A Mighty Fortress Is Our God",
        "Martin Luther",
        """A mighty fortress is our God,
A bulwark never failing;
Our helper He, amid the flood
Of mortal ills prevailing.
For still our ancient foe
Doth seek to work us woe;
His craft and power are great,
And, armed with cruel hate,
On earth is not his equal.

Did we in our own strength confide,
Our striving would be losing,
Were not the right Man on our side,
The Man of God's own choosing.
Dost ask who that may be?
Christ Jesus, it is He;
Lord Sabaoth, His name,
From age to age the same,
And He must win the battle.

And though this world, with devils filled,
Should threaten to undo us,
We will not fear, for God hath willed
His truth to triumph through us.
The Prince of Darkness grim,
We tremble not for him;
His rage we can endure,
For lo, his doom is sure,
One little word shall fell him.

That word above all earthly powers,
No thanks to them, abideth;
The Spirit and the gifts are ours
Through Him who with us sideth.
Let goods and kindred go,
This mortal life also;
The body they may kill:
God's truth abideth still,
His kingdom is forever."""
    ),
    (
        "Because He Lives",
        "Bill & Gloria Gaither",
        """God sent His son, they called Him, Jesus;
He came to love, heal and forgive;
He lived and died to buy my pardon,
An empty grave is there to prove my Savior lives!

Because He lives, I can face tomorrow,
Because He lives, all fear is gone;
Because I know He holds the future,
And life is worth the living,
Just because He lives!

How sweet to hold a newborn baby,
And feel the pride and joy he gives;
But greater still the calm assurance,
This child can face uncertain days because He lives!

And then one day, I'll cross the river,
I'll fight life's final war with pain;
And then, as death gives way to victory,
I'll see the lights of glory and I'll know He lives!"""
    ),
    (
        "Turn Your Eyes Upon Jesus",
        "Helen Lemmel",
        """O soul, are you weary and troubled?
No light in the darkness you see?
There's light for a look at the Savior,
And life more abundant and free!

Turn your eyes upon Jesus,
Look full in His wonderful face,
And the things of earth will grow strangely dim,
In the light of His glory and grace.

Through death into life everlasting,
He passed, and we follow Him there;
Over us sin no more hath dominion,
For more than conquerors we are!

His Word shall not fail you, He promised;
Believe Him, and all will be well:
Then go to a world that is dying,
His perfect salvation to tell!"""
    ),
    (
        "Just as I Am",
        "Charlotte Elliott",
        """Just as I am, without one plea,
But that Thy blood was shed for me,
And that Thou bidst me come to Thee,
O Lamb of God, I come, I come!

Just as I am, and waiting not
To rid my soul of one dark blot,
To Thee whose blood can cleanse each spot,
O Lamb of God, I come, I come!

Just as I am, though tossed about
With many a conflict, many a doubt,
Fightings and fears within, without,
O Lamb of God, I come, I come!

Just as I am, Thou wilt receive,
Wilt welcome, pardon, cleanse, relieve;
Because Thy promise I believe,
O Lamb of God, I come, I come!"""
    ),
    (
        "I Need Thee Every Hour",
        "Annie Hawks",
        """I need Thee every hour, most gracious Lord;
No tender voice like Thine can peace afford.

I need Thee, O I need Thee;
Every hour I need Thee;
O bless me now, my Savior,
I come to Thee.

I need Thee every hour, stay Thou nearby;
Temptations lose their power when Thou art nigh.

I need Thee every hour, in joy or pain;
Come quickly and abide, or life is vain.

I need Thee every hour; teach me Thy will;
And Thy rich promises in me fulfill."""
    ),
    (
        "Sweet Hour of Prayer",
        "William Walford",
        """Sweet hour of prayer! sweet hour of prayer!
That calls me from a world of care,
And bids me at my Father's throne
Make all my wants and wishes known.
In seasons of distress and grief,
My soul has often found relief,
And oft escaped the tempter's snare,
By thy return, sweet hour of prayer!

Sweet hour of prayer! sweet hour of prayer!
Thy wings shall my petition bear
To Him whose truth and faithfulness
Engage the waiting soul to bless.
And since He bids me seek His face,
Believe His Word and trust His grace,
I'll cast on Him my every care,
And wait for thee, sweet hour of prayer!"""
    ),
    (
        "When I Survey the Wondrous Cross",
        "Isaac Watts",
        """When I survey the wondrous cross
On which the Prince of glory died,
My richest gain I count but loss,
And pour contempt on all my pride.

Forbid it, Lord, that I should boast,
Save in the death of Christ my God!
All the vain things that charm me most,
I sacrifice them to His blood.

See from His head, His hands, His feet,
Sorrow and love flow mingled down!
Did e'er such love and sorrow meet,
Or thorns compose so rich a crown?

Were the whole realm of nature mine,
That were a present far too small;
Love so amazing, so divine,
Demands my soul, my life, my all."""
    ),
    (
        "Jesus Paid It All",
        "Elvina Hall",
        """I hear the Savior say,
"Thy strength indeed is small,
Child of weakness, watch and pray,
Find in Me thine all in all."

Jesus paid it all,
All to Him I owe;
Sin had left a crimson stain,
He washed it white as snow.

Lord, now indeed I find
Thy power and Thine alone,
Can change the leper's spots
And melt the heart of stone.

For nothing good have I
Whereby Thy grace to claim;
I'll wash my garments white
In the blood of Calvary's Lamb.

And when, before the throne,
I stand in Him complete,
"Jesus died my soul to save,"
My lips shall still repeat."""
    ),
    (
        "Softly and Tenderly",
        "Will Thompson",
        """Softly and tenderly Jesus is calling,
Calling for you and for me;
See, on the portals He's waiting and watching,
Watching for you and for me.

Come home, come home!
You who are weary, come home!
Earnestly, tenderly, Jesus is calling,
Calling, O sinner, come home!

Why should we tarry when Jesus is pleading,
Pleading for you and for me?
Why should we linger and heed not His mercies,
Mercies for you and for me?

Time is now fleeting, the moments are passing,
Passing from you and from me;
Shadows are gathering, deathbeds are coming,
Coming for you and for me.

O for the wonderful love He has promised,
Promised for you and for me!
Though we have sinned, He has mercy and pardon,
Pardon for you and for me."""
    ),
    (
        "He Lives",
        "Alfred Ackley",
        """I serve a risen Savior, He's in the world today;
I know that He is living, whatever men may say;
I see His hand of mercy, I hear His voice of cheer,
And just the time I need Him He's always near.

He lives, He lives, Christ Jesus lives today!
He walks with me and talks with me along life's narrow way.
He lives, He lives, salvation to impart!
You ask me how I know He lives?
He lives within my heart.

In all the world around me I see His loving care,
And though my heart grows weary I never will despair;
I know that He is leading through all the stormy blast,
The day of His appearing will come at last.

Rejoice, rejoice, O Christian, lift up your voice and sing
Eternal hallelujahs to Jesus Christ the King!
The Hope of all who seek Him, the Help of all who find,
None other is so loving, so good and kind."""
    ),
    (
        "I Surrender All",
        "Judson Van DeVenter",
        """All to Jesus I surrender,
All to Him I freely give;
I will ever love and trust Him,
In His presence daily live.

I surrender all,
I surrender all,
All to Thee, my blessed Savior,
I surrender all.

All to Jesus I surrender,
Humbly at His feet I bow,
Worldly pleasures all forsaken,
Take me, Jesus, take me now.

All to Jesus I surrender,
Make me, Savior, wholly Thine;
Let me feel the Holy Spirit,
Truly know that Thou art mine.

All to Jesus I surrender,
Lord, I give myself to Thee;
Fill me with Thy love and power,
Let Thy blessing fall on me."""
    ),
    (
        "Pass Me Not, O Gentle Savior",
        "Fanny Crosby",
        """Pass me not, O gentle Savior,
Hear my humble cry;
While on others Thou art calling,
Do not pass me by.

Savior, Savior,
Hear my humble cry;
While on others Thou art calling,
Do not pass me by.

Let me at a throne of mercy
Find a sweet relief;
Kneeling there in deep contrition,
Help my unbelief.

Trusting only in Thy merit,
Would I seek Thy face;
Heal my wounded, broken spirit,
Save me by Thy grace.

Thou the Spring of all my comfort,
More than life to me,
Whom have I on earth beside Thee?
Whom in heaven but Thee?"""
    ),
    (
        "Leaning on the Everlasting Arms",
        "Anthony Showalter",
        """What a fellowship, what a joy divine,
Leaning on the everlasting arms;
What a blessedness, what a peace is mine,
Leaning on the everlasting arms.

Leaning, leaning,
Safe and secure from all alarms;
Leaning, leaning,
Leaning on the everlasting arms.

O how sweet to walk in this pilgrim way,
Leaning on the everlasting arms;
O how bright the path grows from day to day,
Leaning on the everlasting arms.

What have I to dread, what have I to fear,
Leaning on the everlasting arms?
I have blessed peace with my Lord so near,
Leaning on the everlasting arms."""
    ),
    (
        "Fairest Lord Jesus",
        "Münster Gesangbuch",
        """Fairest Lord Jesus, Ruler of all nature,
O Thou of God and man the Son,
Thee will I cherish, Thee will I honor,
Thou, my soul's glory, joy, and crown.

Fair are the meadows, fairer still the woodlands,
Robed in the blooming garb of spring;
Jesus is fairer, Jesus is purer,
Who makes the woeful heart to sing.

Fair is the sunshine, fairer still the moonlight,
And all the twinkling starry host;
Jesus shines brighter, Jesus shines purer
Than all the angels heaven can boast.

Beautiful Savior! Lord of all the nations!
Son of God and Son of Man!
Glory and honor, praise, adoration,
Now and forevermore be Thine."""
    ),
    (
        "Victory in Jesus",
        "Eugene Bartlett",
        """I heard an old, old story,
How a Savior came from glory,
How He gave His life on Calvary
To save a wretch like me;
I heard about His groaning,
Of His precious blood's atoning,
Then I repented of my sins
And won the victory.

O victory in Jesus,
My Savior forever!
He sought me and bought me
With His redeeming blood;
He loved me 'ere I knew Him
And all my love is due Him;
He plunged me to victory
Beneath the cleansing flood.

I heard about His healing,
Of His cleansing power revealing,
How He made the lame to walk again
And caused the blind to see;
And then I cried, "Dear Jesus,
Come and heal my broken spirit,"
And somehow Jesus came and brought
To me the victory.

I heard about a mansion
He has built for me in glory,
And I heard about the streets of gold
Beyond the crystal sea;
About the angels singing,
And the old redemption story,
And some sweet day I'll sing up there
The song of victory."""
    ),
    (
        "There Is Power in the Blood",
        "Lewis Jones",
        """Would you be free from the burden of sin?
There's power in the blood, power in the blood;
Would you o'er evil a victory win?
There's wonderful power in the blood.

There is power, power, wonder-working power
In the blood of the Lamb;
There is power, power, wonder-working power
In the precious blood of the Lamb.

Would you be free from your passion and pride?
There's power in the blood, power in the blood;
Come for a cleansing to Calvary's tide;
There's wonderful power in the blood.

Would you be whiter, much whiter than snow?
There's power in the blood, power in the blood;
Sin stains are lost in its life-giving flow;
There's wonderful power in the blood.

Would you do service for Jesus your King?
There's power in the blood, power in the blood;
Would you live daily His praises to sing?
There's wonderful power in the blood."""
    ),
    (
        "Nothing but the Blood of Jesus",
        "Robert Lowry",
        """What can wash away my sin?
Nothing but the blood of Jesus;
What can make me whole again?
Nothing but the blood of Jesus.

Oh! precious is the flow
That makes me white as snow;
No other fount I know,
Nothing but the blood of Jesus.

For my pardon, this I see,
Nothing but the blood of Jesus;
For my cleansing this my plea,
Nothing but the blood of Jesus.

Nothing can for sin atone,
Nothing but the blood of Jesus;
Naught of good that I have done,
Nothing but the blood of Jesus.

This is all my hope and peace,
Nothing but the blood of Jesus;
This is all my righteousness,
Nothing but the blood of Jesus."""
    ),
    (
        "The Solid Rock",
        "Edward Mote",
        """My hope is built on nothing less
Than Jesus' blood and righteousness;
I dare not trust the sweetest frame,
But wholly lean on Jesus' name.

On Christ, the solid Rock, I stand;
All other ground is sinking sand,
All other ground is sinking sand.

When darkness veils His lovely face,
I rest on His unchanging grace;
In every high and stormy gale,
My anchor holds within the veil.

His oath, His covenant, His blood
Support me in the whelming flood;
When all around my soul gives way,
He then is all my hope and stay.

When He shall come with trumpet sound,
Oh, may I then in Him be found;
Dressed in His righteousness alone,
Faultless to stand before the throne."""
    ),
    (
        "Guide Me, O Thou Great Jehovah",
        "William Williams",
        """Guide me, O Thou great Jehovah,
Pilgrim through this barren land;
I am weak, but Thou art mighty;
Hold me with Thy powerful hand;
Bread of heaven, Bread of heaven,
Feed me till I want no more,
Feed me till I want no more.

Open now the crystal fountain,
Whence the healing stream doth flow;
Let the fire and cloudy pillar
Lead me all my journey through;
Strong Deliverer, strong Deliverer,
Be Thou still my strength and shield,
Be Thou still my strength and shield.

When I tread the verge of Jordan,
Bid my anxious fears subside;
Death of death, and hell's Destruction,
Land me safe on Canaan's side;
Songs of praises, songs of praises
I will ever give to Thee,
I will ever give to Thee."""
    ),
    (
        "Praise God From Whom All Blessings Flow",
        "Thomas Ken",
        """Praise God, from whom all blessings flow;
Praise Him, all creatures here below;
Praise Him above, ye heavenly host;
Praise Father, Son, and Holy Ghost. Amen."""
    ),
    (
        "Count Your Blessings",
        "Johnson Oatman",
        """When upon life's billows you are tempest-tossed,
When you are discouraged, thinking all is lost,
Count your many blessings, name them one by one,
And it will surprise you what the Lord has done.

Count your blessings, name them one by one,
Count your blessings, see what God has done!
Count your blessings, name them one by one,
And it will surprise you what the Lord has done.

Are you ever burdened with a load of care?
Does the cross seem heavy you are called to bear?
Count your many blessings, every doubt will fly,
And you will keep singing as the days go by.

When you look at others with their lands and gold,
Think that Christ has promised you His wealth untold;
Count your many blessings—wealth can never buy
Your reward in heaven, nor your home on high."""
    ),
    (
        "Christ the Lord Is Risen Today",
        "Charles Wesley",
        """Christ the Lord is risen today, Alleluia!
Sons of men and angels say, Alleluia!
Raise your joys and triumphs high, Alleluia!
Sing, ye heavens, and earth reply, Alleluia!

Lives again our glorious King, Alleluia!
Where, O death, is now thy sting? Alleluia!
Once He died, our souls to save, Alleluia!
Where thy victory, O grave? Alleluia!

Love's redeeming work is done, Alleluia!
Fought the fight, the battle won, Alleluia!
Death in vain forbids Him rise, Alleluia!
Christ has opened Paradise, Alleluia!

Soar we now where Christ has led, Alleluia!
Following our exalted Head, Alleluia!
Made like Him, like Him we rise, Alleluia!
Ours the cross, the grave, the skies, Alleluia!"""
    ),
    (
        "Joy to the World",
        "Isaac Watts",
        """Joy to the world, the Lord is come!
Let earth receive her King;
Let every heart prepare Him room,
And heaven and nature sing,
And heaven and nature sing,
And heaven, and heaven, and nature sing.

Joy to the world, the Savior reigns!
Let men their songs employ;
While fields and floods, rocks, hills and plains
Repeat the sounding joy,
Repeat the sounding joy,
Repeat, repeat, the sounding joy.

No more let sins and sorrows grow,
Nor thorns infest the ground;
He comes to make His blessings flow
Far as the curse is found,
Far as the curse is found,
Far as, far as, the curse is found.

He rules the world with truth and grace,
And makes the nations prove
The glories of His righteousness,
And wonders of His love,
And wonders of His love,
And wonders, wonders, of His love."""
    ),
    (
        "Hark! The Herald Angels Sing",
        "Charles Wesley",
        """Hark! The herald angels sing,
"Glory to the newborn King;
Peace on earth, and mercy mild,
God and sinners reconciled!"
Joyful, all ye nations rise,
Join the triumph of the skies;
With the angelic host proclaim,
"Christ is born in Bethlehem!"
Hark! the herald angels sing,
"Glory to the newborn King!"

Christ, by highest heaven adored;
Christ, the everlasting Lord;
Late in time, behold Him come,
Offspring of a virgin's womb:
Veiled in flesh the Godhead see;
Hail the incarnate Deity,
Pleased as man with men to dwell,
Jesus, our Emmanuel.
Hark! the herald angels sing,
"Glory to the newborn King!"

Hail the heaven-born Prince of Peace!
Hail the Sun of Righteousness!
Light and life to all He brings,
Risen with healing in His wings.
Mild He lays His glory by,
Born that man no more may die,
Born to raise the sons of earth,
Born to give them second birth.
Hark! the herald angels sing,
"Glory to the newborn King!\""""
    ),
    (
        "Silent Night",
        "Joseph Mohr",
        """Silent night, holy night,
All is calm, all is bright
Round yon virgin mother and child.
Holy Infant, so tender and mild,
Sleep in heavenly peace,
Sleep in heavenly peace.

Silent night, holy night,
Shepherds quake at the sight;
Glories stream from heaven afar,
Heavenly hosts sing Alleluia!
Christ the Savior is born,
Christ the Savior is born!

Silent night, holy night,
Son of God, love's pure light;
Radiant beams from Thy holy face
With the dawn of redeeming grace,
Jesus, Lord, at Thy birth,
Jesus, Lord, at Thy birth."""
    ),

    # ==================== CONTEMPORARY WORSHIP ====================
    (
        "This Is Amazing Grace",
        "Phil Wickham",
        """Who breaks the power of sin and darkness?
Whose love is mighty and so much stronger?
The King of Glory, the King above all kings.

Who shakes the whole earth with holy thunder?
And leaves us breathless in awe and wonder?
The King of Glory, the King above all kings.

This is amazing grace,
This is unfailing love,
That You would take my place,
That You would bear my cross.
You laid down Your life,
That I would be set free.
Oh, Jesus, I sing for
All that You've done for me.

Who brings our chaos back into order?
Who makes the orphan a son and daughter?
The King of Glory, the King of Glory.

Who rules the nations with truth and justice?
Shines like the sun in all of its brilliance?
The King of Glory, the King above all kings.

Worthy is the Lamb who was slain!
Worthy is the King who conquered the grave!
Worthy is the Lamb who was slain!
Worthy is the King who conquered the grave!"""
    ),
    (
        "Lord I Need You",
        "Matt Maher",
        """Lord, I come, I confess,
Bowing here I find my rest.
And without You I fall apart,
You're the one that guides my heart.

Lord, I need You, oh, I need You,
Every hour I need You.
My one defense, my righteousness,
Oh God, how I need You.

Where sin runs deep, Your grace is more,
Where grace is found is where You are.
And where You are, Lord, I am free,
Holiness is Christ in me.

So teach my song to rise to You,
When temptation comes my way.
And when I cannot stand I'll fall on You,
Jesus, You're my hope and stay."""
    ),
    (
        "Lord You Are Good",
        "Israel Houghton",
        """Lord You are good
And Your mercy endureth forever!
Lord You are good
And Your mercy endureth forever!

People from every nation and tongue,
From generation to generation:

We worship You,
Hallelujah, Hallelujah!
We worship You for who You are!
We worship You,
Hallelujah, Hallelujah!
We worship You for who You are!
You are good!

You are good, all the time,
All the time, You are good!"""
    ),
    (
        "Mighty to Save",
        "Hillsong Worship",
        """Everyone needs compassion,
Love that's never failing;
Let mercy fall on me.
Everyone needs forgiveness,
The kindness of a Savior;
The hope of nations.

Savior, He can move the mountains,
My God is mighty to save,
He is mighty to save!
Forever, Author of salvation,
He rose and conquered the grave,
Jesus conquered the grave!

So take me as You find me,
All my fears and failures;
Fill my life again.
I give my life to follow
Everything I believe in,
Now I surrender.

Shine your light and let the whole world see,
We're singing for the glory of the risen King!
Jesus, shine your light and let the whole world see,
We're singing for the glory of the risen King!"""
    ),
    (
        "Here I Am to Worship",
        "Tim Hughes",
        """Light of the world, You stepped down into darkness,
Opened my eyes, let me see
Beauty that made this heart adore You,
Hope of a life spent with You.

Here I am to worship,
Here I am to bow down,
Here I am to say that You're my God.
You're altogether lovely,
Altogether worthy,
Altogether wonderful to me.

King of all days, oh so highly exalted,
Glorious in heaven above;
Humbly You came to the earth You created,
All for love's sake became poor.

And I'll never know how much it cost
To see my sin upon that cross.
And I'll never know how much it cost
To see my sin upon that cross."""
    ),
    (
        "Everlasting God",
        "Brenton Brown",
        """Strength will rise as we wait upon the Lord,
We will wait upon the Lord,
We will wait upon the Lord.
Strength will rise as we wait upon the Lord,
We will wait upon the Lord,
We will wait upon the Lord.

Our God, You reign forever!
Our Hope, our strong Deliverer!

You are the everlasting God,
The everlasting God!
You do not faint, You won't grow weary!
You're the defender of the weak,
You comfort those in need,
You lift us up on wings like eagles!"""
    ),
    (
        "Forever",
        "Chris Tomlin",
        """Give thanks to the Lord, our God and King,
His love endures forever.
For He is good, He is above all things,
His love endures forever.
Sing praise, sing praise!

With a mighty hand and an outstretched arm,
His love endures forever.
For the life that's been reborn,
His love endures forever.
Sing praise, sing praise!

Forever God is faithful,
Forever God is strong,
Forever God is with us,
Forever, forever!

From the rising to the setting sun,
His love endures forever.
And by the grace of God we will carry on,
His love endures forever.
Sing praise, sing praise!"""
    ),
    (
        "We Fall Down",
        "Chris Tomlin",
        """We fall down, we lay our crowns
At the feet of Jesus;
The greatness of mercy and love
At the feet of Jesus.

And we cry holy, holy, holy,
And we cry holy, holy, holy,
And we cry holy, holy, holy is the Lamb."""
    ),
    (
        "Breathe",
        "Marie Barnett",
        """This is the air I breathe,
This is the air I breathe,
Your holy presence living in me.

This is my daily bread,
This is my daily bread,
Your very word spoken to me.

And I, I'm desperate for You,
And I, I'm lost without You."""
    ),
    (
        "Draw Me Close",
        "Kelly Carpenter",
        """Draw me close to You,
Never let me go.
I lay it all down again,
To hear You say that I'm Your friend.

You are my desire,
No one else will do,
'Cause nothing else could take Your place,
To feel the warmth of Your embrace.
Help me find the way,
Bring me back to You.

You're all I want,
You're all I've ever needed.
You're all I want,
Help me know You are near."""
    ),
    (
        "As the Deer",
        "Martin Nystrom",
        """As the deer panteth for the water,
So my soul longeth after Thee.
You alone are my heart's desire,
And I long to worship Thee.

You alone are my strength, my shield;
To You alone may my spirit yield.
You alone are my heart's desire,
And I long to worship Thee.

You're my friend and You are my brother
Even though You are a King.
I love You more than any other,
So much more than anything.

I want You more than gold or silver,
Only You can satisfy.
You alone are the real joy giver
And the apple of my eye."""
    ),
    (
        "Above All",
        "Lenny LeBlanc & Paul Baloche",
        """Above all powers, above all kings,
Above all nature and all created things;
Above all wisdom and all the ways of man,
You were here before the world began.

Above all kingdoms, above all thrones,
Above all wonders the world has ever known;
Above all wealth and treasures of the earth,
There's no way to measure what You're worth.

Crucified, laid behind the stone;
You lived to die, rejected and alone.
Like a rose trampled on the ground,
You took the fall and thought of me,
Above all."""
    ),
    (
        "I Could Sing of Your Love Forever",
        "Delirious?",
        """Over the mountains and the sea,
Your river runs with love for me,
And I will open up my heart
And let the Healer set me free.
I'm happy to be in the truth,
And I will daily lift my hands:
For I will always sing of when Your love came down.

I could sing of Your love forever,
I could sing of Your love forever,
I could sing of Your love forever,
I could sing of Your love forever!

Oh, I feel like dancing,
It's foolishness I know;
But when the world has seen the light,
They will dance with joy like we're dancing now!"""
    ),
    (
        "The Heart of Worship",
        "Matt Redman",
        """When the music fades, all is stripped away,
And I simply come;
Longing just to bring something that's of worth
That will bless Your heart.

I'll bring You more than a song,
For a song in itself
Is not what You have required.
You search much deeper within
Through the way things appear;
You're looking into my heart.

I'm coming back to the heart of worship,
And it's all about You,
It's all about You, Jesus.
I'm sorry, Lord, for the thing I've made it,
When it's all about You,
It's all about You, Jesus.

King of endless worth, no one could express
How much You deserve.
Though I'm weak and poor, all I have is Yours,
Every single breath!"""
    ),
    (
        "Give Thanks",
        "Henry Smith",
        """Give thanks with a grateful heart,
Give thanks to the Holy One,
Give thanks because He's given Jesus Christ, His Son.

Give thanks with a grateful heart,
Give thanks to the Holy One,
Give thanks because He's given Jesus Christ, His Son.

And now let the weak say, "I am strong,"
Let the poor say, "I am rich
Because of what the Lord has done for us."

And now let the weak say, "I am strong,"
Let the poor say, "I am rich
Because of what the Lord has done for us."
Give thanks!"""
    ),
    (
        "Trading My Sorrows",
        "Darrell Evans",
        """I'm trading my sorrows,
I'm trading my shame;
I'm laying them down for the joy of the Lord.

I'm trading my sickness,
I'm trading my pain;
I'm laying them down for the joy of the Lord.

Yes, Lord, yes, Lord, yes, yes, Lord!
Yes, Lord, yes, Lord, yes, yes, Lord!
Yes, Lord, yes, Lord, yes, yes, Lord, Amen!

I am pressed but not crushed, persecuted not abandoned,
Struck down but not destroyed.
I am blessed beyond the curse, for His promise will endure,
And His joy's gonna be my strength.

Though the sorrow may last for the night,
His joy comes with the morning!"""
    ),
    (
        "Days of Elijah",
        "Robin Mark",
        """These are the days of Elijah,
Declaring the Word of the Lord;
And these are the days of Your servant, Moses,
Righteousness being restored.
And though these are days of great trial,
Of famine and darkness and sword;
Still, we are the voice in the desert crying,
"Prepare ye the way of the Lord!"

Behold He comes, riding on the clouds,
Shining like the sun at the trumpet call;
Lift your voice, it's the year of Jubilee,
And out of Zion's hill salvation comes!

And these are the days of Ezekiel,
The dry bones becoming as flesh;
And these are the days of Your servant, David,
Rebuilding a temple of praise.
And these are the days of the harvest,
The fields are as white in the world;
And we are the laborers in Your vineyard,
Declaring the Word of the Lord!"""
    ),
    (
        "You Are My All in All",
        "Dennis Jernigan",
        """You are my strength when I am weak,
You are the treasure that I seek,
You are my all in all.
Seeking You as a precious jewel,
Lord, to give up I'd be a fool,
You are my all in all.

Jesus, Lamb of God,
Worthy is Your name!
Jesus, Lamb of God,
Worthy is Your name!

Taking my sin, my cross, my shame,
Rising again I bless Your name,
You are my all in all.
When I fall down You pick me up,
When I am dry You fill my cup,
You are my all in all."""
    ),
    (
        "O Praise the Name (Anástasis)",
        "Hillsong Worship",
        """I cast my mind to Calvary
Where Jesus bled and died for me.
I see His wounds, His hands, His feet,
My Savior on that cursed tree.

His body bound and drenched in tears
They laid Him down in Joseph's tomb.
The entrance sealed by heavy stone
Messiah still and all alone.

O praise the Name of the Lord our God!
O praise His Name forevermore!
For endless days we will sing Your praise,
Oh Lord, oh Lord our God!

Then on the third at break of dawn,
The Son of heaven rose again.
O trampled death, where is your sting?
The angels roar for Christ the King!

He shall return in robes of white,
The blazing sun shall pierce the night.
And I will rise among the saints,
My gaze transfixed on Jesus' face."""
    ),
    (
        "Graves Into Gardens",
        "Elevation Worship",
        """I searched the world but it couldn't fill me,
A man's empty praise and treasures that fade
Are never enough.
Then You came along and You put me back together,
And every desire is now satisfied
Here in Your love.

Oh, there's nothing better than You,
There's nothing better than You!
Lord, there's nothing, nothing is better than You!

I'm not afraid to show You my weakness,
My failures and flaws, Lord, You've seen them all
And You still call me friend.
'Cause the God of the mountain is the God of the valley,
There's not a place Your mercy and the grace
Won't find me again.

You turn mourning to dancing,
You give beauty for ashes,
You turn shame into glory,
You're the only one who can!

You turn graves into gardens,
You turn bones into armies,
You turn seas into highways,
You're the only one who can!"""
    ),
    (
        "Praise",
        "Elevation Worship",
        """Let everything that has breath
Praise the Lord!
Praise the Lord!

I'll praise in the valley,
Praise on the mountain!
I'll praise when I'm sure,
Praise when I'm doubting!
I'll praise when outnumbered,
Praise when surrounded,
'Cause praise is the water
My enemies drown in!

As long as I'm breathing,
I've got a reason to
Praise the Lord, oh my soul!
Praise the Lord, oh my soul!

I'll praise when I feel it,
And I'll praise when I don't!
I'll praise 'cause I know
You're still in control!
My praise is a weapon,
It's more than a sound,
My praise is the shout
That brings Jericho down!"""
    ),
    (
        "Do It Again",
        "Elevation Worship",
        """Walking around these walls
I thought by now they'd fall,
But You have never failed me yet.
Waiting for change to come,
Knowing the battle's won,
For You have never failed me yet.

Your promise still stands,
Great is Your faithfulness,
Faithfulness!
I'm still in Your hands,
This is my confidence,
You've never failed me yet!

I know the night won't last,
Your word will come to pass,
My heart will sing Your praise again.
Jesus, You're still enough,
Keep me within Your love,
My heart will sing Your praise again.

I've seen You move, You move the mountains,
And I believe I'll see You do it again!
You made a way, where there was no way,
And I believe I'll see You do it again!"""
    ),
    (
        "Jireh",
        "Elevation Worship & Maverick City Music",
        """I'll never be more loved than I am right now,
Wasn't holding You up, so there's nothing I can do to let You down.
It doesn't take a trophy to make You proud,
I'll never be more loved than I am right now.

Going through a storm but I won't go down,
I hear Your voice carried in the rhythm of the wind to call me out.
You would cross an ocean so I wouldn't drown,
You've never been closer than You are right now.

Jireh, You are enough!
Jireh, You are enough!
And I will be content in every circumstance,
Jireh, You are enough!

Forever enough,
Always enough,
More than enough!

If He dresses the lilies with beauty and splendor,
How much more will He clothe you?
How much more will He clothe you?
If He watches over every sparrow,
How much more does He love you?
How much more does He love you?"""
    ),

    # ==================== GLOBAL & AFRICAN GOSPEL CLASSICS ====================
    (
        "Ancient of Days",
        "Ron Kenoly",
        """Blessing and honor, glory and power,
Be unto the Ancient of Days;
From every nation, all of creation,
Bow before the Ancient of Days.

Every tongue in heaven and earth shall declare Your glory,
Every knee shall bow at Your throne in worship;
You will be exalted, O God, and Your kingdom shall not pass away,
O Ancient of Days!

Your kingdom shall reign over all the earth,
Sing unto the Ancient of Days;
For none can compare to Your matchless worth,
Praise unto the Ancient of Days."""
    ),
    (
        "You Are Alpha and Omega",
        "Israel Houghton",
        """You are Alpha and Omega,
We worship You, our Lord,
You are worthy to be praised.

You are Alpha and Omega,
We worship You, our Lord,
You are worthy to be praised.

We give You all the glory,
We worship You, our Lord,
You are worthy to be praised."""
    ),
    (
        "Imela",
        "Nathaniel Bassey & Enitan Adaba",
        """When I consider the work of Your hands,
The sun, the moon, and the stars in their courses,
What is man that You are mindful of him?
And the son of man that You visit him so?

Imela, Imela,
Eze m Oh, Imela!
(Thank You, Thank You, My King, Thank You!)
Imela, Imela,
Eze m Oh, Imela!

When I was drowning in the sea of sin and shame,
You reached down Your hand and You lifted me up;
You set my feet upon the Rock to stay,
And put a new song in my mouth today!"""
    ),
    (
        "Onise Iyanu",
        "Nathaniel Bassey",
        """Onise Iyanu, You're the God of awesome wonders,
I've tasted of Your power.
Onise Iyanu, You have shown me so much mercy,
Much more than I deserve.

My Father, my Father,
No one compares to You!
The God who makes a way,
Where there was no way!

Onise Iyanu,
God of awesome wonders!
Onise Iyanu,
God of awesome wonders!"""
    ),
    (
        "Olowogbogboro",
        "Nathaniel Bassey",
        """Olowogbogboro is turning things around for my good!
Olowogbogboro is turning things around for my good!
The outstretched arm of God,
Has brought deliverance to me!
Olowogbogboro is turning things around for my good!

He has stretched out His hand,
And who can turn it back?
He has spoken His word,
And it will surely come to pass!"""
    ),
    (
        "Nara",
        "Tim Godfrey & Travis Greene",
        """What shall I render to Jehovah?
For He has done so very much for me.
What shall I render to Jehovah?
For He has done so very much for me.

Nara, nara ekele,
Nar'ekele me, Onye di nso!
(Receive, receive thanksgiving,
Receive my thanksgiving, Holy One!)
Nara, nara ekele,
Nar'ekele me, Onye di nso!

You've done so much for me,
I cannot tell it all,
Narekele mo!
If I had ten thousand tongues,
It still wouldn't be enough,
Narekele mo!"""
    ),
    (
        "Excess Love",
        "Mercy Chinwo",
        """Your love is kind, Your love is patient,
You fill my heart with so much peace and joy!
You're amazing, You make my life feel brand new!
Jesus, You're amazing, You make my life feel brand new!

Jesus, You love me too much o,
Too much o, too much o, excess love o!
Jesus, You love me too much o,
Too much o, too much o, excess love o!

All around, You make my life so beautiful,
Like a brand new day,
Your love is real to me!"""
    ),
    (
        "Chinedum",
        "Mercy Chinwo",
        """Anywhere You lead me, I will go,
'Cause You're the way, truth and the life, hey!
Jesus, You'll never ever let me down,
All of my trust is in You.

Chinedum, Chinedum,
(God leads me, God leads me)
Every step of the way,
Chinedum o!

You take me by the hand,
You lead me to the promised land,
Nothing can separate me from Your love!"""
    ),
    (
        "Eze Ebube",
        "Neon Adejo",
        """When I look back over my life,
And I see where You brought me from,
I stand in awe of Your goodness, Lord,
And my heart overflows with praise.

Eze Ebube, see how far You've brought me!
Eze Ebube, I am so grateful!
Eze Ebube, see how far You've brought me!
I am so grateful for Your grace!"""
    ),
    (
        "You Are Great",
        "Steve Crown",
        """You are great, yes You are, Holy one;
Walked upon the sea, raised the dead;
Reign in majesty, mighty God,
Everything written about You is great!

You are great,
You are great,
You are great,
You are great!

Everything written about You is great,
Everything written about You is great!"""
    ),
    (
        "God All by Yourself",
        "Eben",
        """You are God from beginning to the end,
There's no place for argument,
You are God all by Yourself!

You are God from beginning to the end,
There's no place for argument,
You are God all by Yourself!

No one can compare to Your name,
No one can challenge Your throne,
You are God all by Yourself!"""
    ),
    (
        "Awesome God",
        "Rich Mullins",
        """When He rolls up His sleeves
He ain't just putting on the ritz,
Our God is an awesome God!
There is thunder in His footsteps
And lightning in His fists,
Our God is an awesome God!

Our God is an awesome God,
He reigns from heaven above
With wisdom, power, and love,
Our God is an awesome God!

And when the sky was starless
In the void of the night,
Our God is an awesome God!
He spoke into the darkness
And created the light,
Our God is an awesome God!"""
    ),
    (
        "No Longer Slaves",
        "Bethel Music",
        """You unravel me with a melody,
You surround me with a song
Of deliverance from my enemies
Till all my fears are gone.

I'm no longer a slave to fear,
I am a child of God!
I'm no longer a slave to fear,
I am a child of God!

From my Mother's womb
You have chosen me,
Love has called my name!
I've been born again into Your family,
Your blood flows through my veins!

You split the sea so I could walk right through it,
My fears were drowned in perfect love!
You rescued me so I could stand and sing:
I am a child of God!"""
    ),
    (
        "Tobechukwu",
        "Nathaniel Bassey & Mercy Chinwo",
        """Even before I pray, Jehovah Nissi has answered me!
He has broken every chain,
And given me victory!

Tobechukwu, Tobechukwu,
(Praise God, Praise God)
Praise the Lord for His goodness!
Tobechukwu, Tobechukwu,
Praise the Lord for His grace!"""
    ),
    (
        "You Deserve the Glory",
        "Terry MacAlmon",
        """You deserve the glory and the honor,
Lord, we lift our hands in worship,
As we lift Your holy name.

You deserve the glory and the honor,
Lord, we lift our hands in worship,
As we lift Your holy name.

For You are great, You do miracles so great,
There is no one else like You,
There is no one else like You!
For You are great, You do miracles so great,
There is no one else like You,
There is no one else like You!"""
    ),
    (
        "Yahweh",
        "Koffi Owusu",
        """You are the God who does impossible things,
From age to age Your praise will ring.
We lift our voices high in praise,
To the King of endless days!

Yahweh, Yahweh,
We worship You today!
Yahweh, Yahweh,
Forever You will reign!"""
    ),
    (
        "Bigger Everyday",
        "Moses Bliss",
        """Jesus, You make my life so sweet,
Every single day I am blessed!
Walking in Your favor, walking in Your grace,
Glory and honor to Your holy name!

Bigger everyday,
Higher everyday,
Jesus, You're taking me higher everyday!"""
    )
,
    (
        "Amazing Love (You Are My King)",
        "Billy James Foote",
        """I'm forgiven because You were forsaken,
I'm accepted, You were condemned;
I'm alive and well, Your Spirit is within me,
Because You died and rose again.

Amazing love, how can it be,
That You, my King, should die for me?
Amazing love, I know it's true,
And it's my joy to honor You,
In all I do I honor You.

You are my King,
You are my King,
Jesus, You are my King,
Jesus, You are my King!"""
    ),
    (
        "Blessed Be Your Name",
        "Matt Redman",
        """Blessed be Your name
In the land that is plentiful,
Where Your streams of abundance flow,
Blessed be Your name.

Blessed be Your name
When I'm found in the desert place,
Though I walk through the wilderness,
Blessed be Your name.

Every blessing You pour out I'll
Turn back to praise;
When the darkness closes in, Lord,
Still I will say:

Blessed be the name of the Lord,
Blessed be Your name;
Blessed be the name of the Lord,
Blessed be Your glorious name!

Blessed be Your name
When the sun's shining down on me,
When the world's 'all as it should be,'
Blessed be Your name.

Blessed be Your name
On the road marked with suffering,
Though there's pain in the offering,
Blessed be Your name.

You give and take away,
You give and take away;
My heart will choose to say:
Lord, blessed be Your name!"""
    ),
    (
        "Agnus Dei",
        "Michael W. Smith",
        """Alleluia, Alleluia,
For the Lord God Almighty reigns!
Alleluia, Alleluia,
For the Lord God Almighty reigns!
Alleluia!

Holy, Holy,
Are You Lord God Almighty;
Worthy is the Lamb,
Worthy is the Lamb!

You are holy,
Holy,
Are You Lord God Almighty;
Worthy is the Lamb,
Worthy is the Lamb,
Amen!"""
    ),
    (
        "Better Is One Day",
        "Matt Redman",
        """How lovely is Your dwelling place,
O Lord Almighty!
For my soul longs and even faints for You.
For here my heart is satisfied,
Within Your presence;
I sing beneath the shadow of Your wings.

Better is one day in Your courts,
Better is one day in Your house,
Better is one day in Your courts,
Than thousands elsewhere!

One thing I ask and I would seek,
To see Your beauty,
To find You in the place Your glory dwells.
One thing I ask and I would seek,
To see Your beauty,
To find You in the place Your glory dwells.

My heart and flesh cry out
For You, the living God;
Your Spirit's water to my soul.
I've tasted and I've seen,
Come once again to me;
I will draw near to You,
I will draw near to You!"""
    ),
    (
        "Here As In Heaven",
        "Elevation Worship",
        """The atmosphere is changing now,
For the Spirit of the Lord is here;
The evidence is all around,
That the Spirit of the Lord is here.

Overflow in this place,
Fill our hearts with Your love;
Your love, surrounds us.
You're the reason we came,
To encounter Your love;
Your love, surrounds us.

Spirit of God, fall fresh on us,
We need Your presence;
How we need You Lord.
Jesus, be manifest here,
Your glory and Your power,
Here as in heaven!

A miracle can happen now,
For the Spirit of the Lord is here;
The evidence is all around,
That the Spirit of the Lord is here."""
    ),
    (
        "Same God",
        "Elevation Worship",
        """I'm calling on the God of Jacob,
Whose love endures through generations;
I know that You will keep Your covenant.
I'm calling on the God of Moses,
The one who opened up the ocean;
I need You now to do the same thing for me.

O God, my God, I need You!
O God, my God, I need You now!
How I need You now!
O Rock, O Rock of ages,
I'm standing on Your faithfulness!

I'm calling on the God of Mary,
Whose favor rests upon the lowly;
You call the simple things to lead the world.
I'm calling on the God of David,
Who made a shepherd boy a king;
I may be small but You are mighty!

You heard Your children then,
And You will not fail now;
You are the same God,
You are the same God!
You answered prayers back then,
And You will not fail now;
You are the same God,
You are the same God!"""
    ),
    (
        "Good Good Father",
        "Chris Tomlin",
        """I've heard a thousand stories
Of what they think You're like;
But I've heard the tender whisper
Of love in the dead of night.
You tell me that You're pleased
And that I'm never alone.

You're a good, good Father,
It's who You are, it's who You are, it's who You are;
And I'm loved by You,
It's who I am, it's who I am, it's who I am.

I've seen many searching for answers
Far and wide;
But I know we're all searching for answers
Only You provide.
'Cause You know just what we need
Before we say a word.

Because You are perfect in all of Your ways,
You are perfect in all of Your ways,
You are perfect in all of Your ways to us!"""
    ),
    (
        "Holy Spirit",
        "Bryan & Katie Torwalt",
        """There's nothing worth more that will ever come close,
No thing can compare, You're our living hope;
Your Presence, Lord.

I've tasted and seen of the sweetest of loves,
Where my heart becomes free and my shame is undone;
In Your Presence, Lord.

Holy Spirit, You are welcome here,
Come flood this place and fill the atmosphere;
Your glory, God, is what our hearts long for,
To be overcome by Your Presence, Lord.

Let us become more aware of Your Presence,
Let us experience the glory of Your goodness!"""
    ),
    (
        "Lord, I Lift Your Name on High",
        "Rick Founds",
        """Lord, I lift Your name on high,
Lord, I love to sing Your praises;
I'm so glad You're in my life,
I'm so glad You came to save us.

You came from heaven to earth to show the way,
From the earth to the cross, my debt to pay;
From the cross to the grave,
From the grave to the sky,
Lord, I lift Your name on high!"""
    ),
    (
        "Sanctuary",
        "Randy Scruggs & John Thompson",
        """Lord, prepare me to be a sanctuary,
Pure and holy, tried and true;
With thanksgiving, I'll be a living
Sanctuary for You.

It is You, Lord, that I came to seek,
In Your presence is where I belong;
Lord, prepare me, pure and holy,
Sanctuary for You."""
    ),
    (
        "More Than Gold",
        "Judikay",
        """Your presence is more than gold to me,
Your love is sweeter than wine;
Jesus, You're everything I need,
Forever and always mine.

More than gold, more than silver,
More than anything this world can give;
Jesus, You satisfy my soul,
Forever in You I live.

Lord, I give You all the glory,
All the honor and the praise;
For Your grace has rewritten my story,
Blessed be Your holy name!"""
    ),
    (
        "Kumama Papa",
        "Grace Lokwa",
        """Yahweh, Yahweh,
Kumama Papa, kumama!
Yahweh, Yahweh,
Kumama Papa, kumama!

Be lifted high, be glorified,
Kumama Papa, kumama!
For all You've done and who You are,
Kumama Papa, kumama!

You are faithful, You are kind,
Kumama Papa, kumama!
Receive all praise, receive all honor,
Kumama Papa, kumama!"""
    ),
    (
        "Casting Crowns",
        "Nathaniel Bassey",
        """Casting crowns, lifting hands,
Bowing hearts, is all we've come to do;
Casting crowns, lifting hands,
Bowing hearts, is all we've come to do.

Adonai, Adonai,
Adonai, we worship You;
Adonai, Adonai,
Adonai, we worship You.

You are the King of kings,
The Lord of lords,
None can compare to You;
Adonai, Adonai,
We worship You!"""
    ),
    (
        "This Kingdom",
        "Geoff Bullock",
        """Jesus, God's righteousness revealed,
The Son of Man, the Son of God,
His Kingdom comes.
Jesus, redemption's sacrifice,
Now glorified, now justified,
His Kingdom comes.

And this Kingdom will know no end,
And its glory shall know no bounds;
For the majesty and power
Of this Kingdom's King has come.
And this Kingdom's rule, and this Kingdom's reign,
And this Kingdom's power and authority,
Jesus, God's righteousness revealed.

Jesus, the expression of God's love,
The grace of God, the Word of God,
Revealed to us.
Jesus, God's holiness displayed,
Now glorified, now justified,
His Kingdom comes."""
    ),
    (
        "Shout to the North",
        "Martin Smith",
        """Men of faith, rise up and sing
Of the great and glorious King;
You are strong when you feel weak,
In your brokenness complete.

Shout to the north and the south,
Sing to the east and the west:
Jesus is Savior to all,
Lord of heaven and earth!

Rise up, women of the truth,
Stand and sing to broken hearts;
Who can know the healing power
Of our awesome King of love.

We've been through fire, we've been through rain,
We've been refined by the power of His name;
We've fallen deeper in love with You,
You've burned the truth on our lips!"""
    ),
    (
        "Worthy Is the Lamb",
        "Darlene Zschech",
        """Thank You for the cross, Lord,
Thank You for the price You paid;
Bearing all my sin and shame,
In love You came and gave amazing grace.

Thank You for this love, Lord,
Thank You for the nail-pierced hands;
Washed me in Your cleansing flow,
Now all I know Your forgiveness and embrace.

Worthy is the Lamb
Seated on the throne;
Crown You now with many crowns,
You reign victorious!

High and lifted up,
Jesus, Son of God,
The Darling of heaven crucified;
Worthy is the Lamb,
Worthy is the Lamb!"""
    ),
    (
        "Eagle's Wings",
        "Reuben Morgan",
        """Here I am waiting,
Abide in me, I pray;
Here I am longing for You.
Hide me in Your love,
Bring me to my knees;
May I know Jesus more and more.

Come live in me,
All my life take over;
Come breathe in me,
And I will rise on eagle's wings!"""
    ),
    (
        "Hosanna",
        "Brooke Fraser",
        """I see the King of glory
Coming on the clouds with fire;
The whole earth shakes, the whole earth shakes.
I see His love and mercy
Washing over all our sin;
The people sing, the people sing:

Hosanna, Hosanna,
Hosanna in the highest!
Hosanna, Hosanna,
Hosanna in the highest!

I see a generation
Rising up to take their place
With selfless faith, with selfless faith.
I see a near revival
Stirring as we pray and seek;
We're on our knees, we're on our knees.

Heal my heart and cleanse it,
Open up my eyes to the things unseen;
Show me how to love like You have loved me.
Break my heart for what breaks Yours,
Everything I am for Your Kingdom's cause,
As I walk from earth into eternity!"""
    ),
    (
        "Jesus at the Center",
        "Israel Houghton",
        """Jesus at the center of it all,
Jesus at the center of it all;
From beginning to the end,
It will always be, it's always been You,
Jesus, Jesus.

Nothing else matters,
Nothing in this world will do;
'Cause Jesus, You're the center,
Everything revolves around You,
Jesus, You.

From my heart to the heavens,
Jesus be the center;
It's all about You,
Yes, it's all about You!"""
    ),
    (
        "There Is None Like You",
        "Lenny LeBlanc",
        """There is none like You,
No one else can touch my heart like You do;
I could search for all eternity long, and find,
There is none like You.

Your mercy flows like a river wide,
And healing comes from Your hands;
Suffering children are safe in Your arms,
There is none like You."""
    ),
    (
        "All to Us",
        "Chris Tomlin",
        """Precious cornerstone, sure foundation,
You are faithful to the end;
We are waiting on You, Jesus,
We believe You're all to us.

Let the glory of Your name
Be the passion of the church;
Let the righteousness of God
Be a holy flame that burns.
Let the saving love of Christ
Be the measure of our lives,
We believe You're all to us!

Only Son of God, sent from heaven,
Hope and mercy at the cross;
You are everything, You're the Promise,
Jesus, You are all to us!"""
    ),
    (
        "Trust in God",
        "Elevation Worship",
        """Blessed assurance, Jesus is mine!
He's been my fourth man in the fire, time after time;
Born of His Spirit, washed in His blood,
And what He did for me on Calvary is more than enough!

I trust in God,
My Savior, the one who will never fail;
He will never fail!
I trust in God,
My Savior, the one who will never fail;
He will never fail!

Perfect submission, all is at rest,
I know the author of tomorrow has ordered my steps;
So this is my story and this is my song,
Praising my Risen King and Savior all the day long!

I sought the Lord and He heard, and He answered!
That's why I trust Him, that's why I trust Him!"""
    ),
    (
        "I Exalt Thee",
        "Pete Sanchez Jr.",
        """For Thou, O Lord, art high above all the earth,
Thou art exalted far above all gods;
For Thou, O Lord, art high above all the earth,
Thou art exalted far above all gods.

I exalt Thee, I exalt Thee,
I exalt Thee, O Lord!
I exalt Thee, I exalt Thee,
I exalt Thee, O Lord!"""
    ),
    (
        "At the Cross (Love Ran Red)",
        "Chris Tomlin",
        """There's a place where mercy reigns and never dies,
There's a place where streams of grace flow deep and wide;
Where all the love I've ever found,
Comes rushing over me like a flood.

At the cross, at the cross,
I surrender my life;
I'm in awe of You, I'm in awe of You.
Where Your love ran red, and my sin washed white,
I owe all to You, I owe all to You,
Jesus!

There's a place where sin and shame are powerless,
Where my heart has peace with God and forgiveness;
Where all the love I've ever found,
Comes rushing over me like a flood.

Here my hope is found,
Here on holy ground;
Here I bow down,
Here I bow down!"""
    ),
    (
        "What a Mighty God We Serve",
        "Traditional",
        """What a mighty God we serve,
What a mighty God we serve!
Angels bow before Him,
Heaven and earth adore Him,
What a mighty God we serve!

O Zion, O Zion, that bringest good tidings,
Get thee up into the high mountain;
Jerusalem, that bringest good tidings,
Lift up thy voice with strength!

What a mighty God we serve,
What a mighty God we serve!
Angels bow before Him,
Heaven and earth adore Him,
What a mighty God we serve!"""
    ),
    (
        "You Are Good",
        "Bethel Music",
        """I want to scream it out
From every mountain top;
Your goodness has no end,
It will never stop.
Your mercy comes like a wave,
Crashing over me,
For You have set me free!

'Cause You are good, You are good,
You are good to me!
'Cause You are good, You are good,
You are good to me!

With every breath I breathe,
I will sing of Your praise;
You took my brokenness,
And turned it into grace.
Forever I will declare,
You are good!"""
    ),
    (
        "Jehovah You I Know",
        "Eben",
        """Jehovah You I know,
Jehovah You I see;
Jehovah You I feel,
You're everything to me.

There is none like You in all the earth,
There is none like You, O Lord;
You are the covenant-keeping God,
Faithful from age to age.

Jehovah You I know,
Jehovah You I see;
Jehovah You I feel,
You're everything to me!"""
    ),
    (
        "My Tribute (To God Be the Glory)",
        "Andraé Crouch",
        """How can I say thanks
For the things You have done for me?
Things so undeserved,
Yet You gave to prove Your love for me;
The voices of a million angels
Could not express my gratitude.
All that I am and ever hope to be,
I owe it all to Thee.

To God be the glory,
To God be the glory,
To God be the glory,
For the things He has done!

With His blood He has saved me,
With His power He has raised me;
To God be the glory,
For the things He has done!

Just let me live my life,
Let it be pleasing, Lord to Thee;
And should I gain any praise,
Let it go to Calvary!"""
    )
]

def main():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    # Ensure schema
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS lyrics (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            artist TEXT,
            content TEXT NOT NULL,
            source TEXT DEFAULT 'local',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            normalized_key TEXT,
            content_hash TEXT
        );
    """)
    cursor.execute("""
        CREATE VIRTUAL TABLE IF NOT EXISTS lyrics_fts USING fts5(
            title,
            artist,
            content,
            content='lyrics',
            content_rowid='id'
        );
    """)

    added = 0
    skipped = 0

    for title, artist, content in SONGS:
        clean_content = content.strip()
        norm_key = normalize_key(title, artist)
        content_hash = clean_content[:64]

        # Check existing
        cursor.execute("SELECT id FROM lyrics WHERE lower(title) = ? AND lower(ifnull(artist,'')) = ?",
                       (title.lower(), (artist or '').lower()))
        row = cursor.fetchone()
        if row:
            skipped += 1
            continue

        cursor.execute("""
            INSERT INTO lyrics (title, artist, content, source, normalized_key, content_hash)
            VALUES (?, ?, ?, 'Bundled Hymnal & Gospel Collection', ?, ?)
        """, (title, artist, clean_content, norm_key, content_hash))
        
        song_id = cursor.lastrowid
        cursor.execute("""
            INSERT INTO lyrics_fts (rowid, title, artist, content)
            VALUES (?, ?, ?, ?)
        """, (song_id, title, artist, clean_content))
        added += 1

    conn.commit()

    cursor.execute("SELECT COUNT(*) FROM lyrics")
    total = cursor.fetchone()[0]

    cursor.execute("SELECT COUNT(*) FROM lyrics_fts")
    fts_total = cursor.fetchone()[0]

    conn.close()
    print(f"Seeding complete! Added: {added}, Skipped: {skipped}, Total in Library: {total}, Total in FTS index: {fts_total}")

if __name__ == "__main__":
    main()
