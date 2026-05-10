const { mdToPdf } = require('/opt/ext1/lzh/.agents/skills/markdown-to-pdf/node_modules/md-to-pdf/dist');

async function main() {
  const pdf = await mdToPdf(
    { path: '/home/lzh/CSE5030/lab11/report.md' },
    {
      dest: '/home/lzh/CSE5030/lab11/report.pdf',
      stylesheet: '/home/lzh/CSE5030/lab11/report-pdf.css',
      pdf_options: {
        format: 'A4',
        margin: {
          top: '1.2cm',
          bottom: '1.2cm',
          left: '1.1cm',
          right: '1.1cm'
        },
        printBackground: true
      },
      highlight_style: 'github',
      marked_options: {
        gfm: true,
        breaks: false
      }
    }
  );

  if (!pdf || !pdf.filename) {
    throw new Error('PDF export failed');
  }

  console.log(JSON.stringify({
    success: true,
    output: pdf.filename
  }));
}

main().catch((error) => {
  console.error(JSON.stringify({
    success: false,
    error: error.message
  }));
  process.exit(1);
});
