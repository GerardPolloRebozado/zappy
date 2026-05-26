#let rfc(
  title: "Untitled RFC",
  authors: (
    "Author Name",
  ),
  rfc-number: "XXXX",
  category: "Informational",
  date: datetime.today().display("[month repr:long] [year]"),
  group: "Network Working Group",
  body,
) = {
  let footer-authors = authors.join(", ")

  set page(
    paper: "us-letter",
    margin: (x: 1in, y: 1in),
    header: [
      #set text(font: ("Courier", "Liberation Mono", "DejaVu Sans Mono", "Noto Sans Mono"), size: 10pt)
      RFC #rfc-number
      #h(1fr)
      #title
      #h(1fr)
      #date
      #v(-0.5em)
      #line(length: 100%, stroke: 0.5pt)
    ],
    footer: [
      #line(length: 100%, stroke: 0.5pt)
      #v(-0.5em)
      #set text(font: ("Courier", "Liberation Mono", "DejaVu Sans Mono", "Noto Sans Mono"), size: 10pt)
      #footer-authors
      #h(1fr)
      Page  #context counter(page).display()
    ],
  )

  // --- TYPOGRAPHY ---
  set text(font: ("Courier", "Liberation Mono", "DejaVu Sans Mono", "Noto Sans Mono"), size: 10pt)
  set par(justify: false, leading: 0.65em)
  show par: set block(spacing: 1.5em)
  set heading(numbering: "1.1.  ")
  show heading: it => {
    set text(weight: "bold", size: 10pt) 
    v(1em)
    it
    v(0.5em)
  }

  show regex("([.!?]) +"): it => {
    it.text.at(0)
    h(0.5em, weak: true)
    "  "
  }

  grid(
    columns: (1fr, 1fr),
    [
      #group \
      Request for Comments: #rfc-number \
      Category: #category
    ],
    align(right)[
      #for author in authors [
        #author \
      ]
      #date
    ],
  )

  v(4em)

  align(center)[
    #block(width: 80%)[
      #set text(weight: "bold", size: 12pt)
      #upper(title)
    ]
  ]

  v(2em)

  outline(
    title: "Table of Contents",
    indent: 2em,
  )

  pagebreak(weak: true)

  show par: pad.with(left: 3em)
  show list: pad.with(left: 3em)
  show enum: pad.with(left: 3em)
  show raw.where(block: true): pad.with(left: 3em)

  body
}
